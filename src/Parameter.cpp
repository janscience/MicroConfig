#include <Menu.h>
#include <Parameter.h>


Parameter::Parameter(Menu &menu, const char *name, size_t n) :
  Action(menu, name, SetValue | AllRoles),
  NSelection(n) {
  TypeStr[0] = '\0';
}


void Parameter::write(Stream &stream, unsigned int roles, size_t indent,
		      size_t width, bool descend) const {
  if (enabled(roles)) {
    char pval[MaxVal];
    valueStr(pval);
    size_t kw = width >= strlen(name()) ? width - strlen(name()) : 0;
    stream.printf("%*s%s:%*s %s\n", indent, "", name(), kw, "", pval);
  }
}


void Parameter::execute(Stream &stream) {
  if (disabled(StreamIO))
    return;
  if (disabled(SetValue)) {
    write(stream, StreamIO);
    stream.println();
    return;
  }
  int w = strlen(name());
  if (w < 16)
    w = 16;
  char pval[MaxVal];
  valueStr(pval);
  stream.printf("%-*s: %s\n", w, name(), pval);
  listSelection(stream);
  while (true) {
    if (NSelection > 0)
      stream.print("Select new value");
    else
      stream.print("Enter new value ");
    instructions(pval);
    if (strlen(pval) > 0)
      stream.printf(" (%s)", pval);
    stream.print(": ");
    while (stream.available() == 0) {
      yield();
      delay(1);
    }
    stream.readBytesUntil('\n', pval, MaxVal);
    if ((strcmp(pval, "ktv") == 0) ||
	(strcmp(pval, "keepthevalue") == 0) ||
	parseValue(pval, NSelection > 0)) {
      if (echo())
	stream.println(pval);
      break;
    }
    if (echo())
      stream.println(pval);
  }
  stream.println();
}


void Parameter::set(const char *val, const char *name, Stream &stream) {
  size_t kn = 0;
  if (name != 0)
    kn = strlen(name);
  char keyname[kn + strlen(this->name()) + 1];
  keyname[0] = '\0';
  if (kn > 0) {
    strcat(keyname, name);
    strcat(keyname, ">");
  }
  strcat(keyname, this->name());
  if (disabled(SetValue)) {
    if (enabled(StreamOutput))
      stream.printf("%*ssetting a new value for %s is disabled\n",
		    indentation(), "", keyname);
    return;
  }
  char pval[MaxVal];
  strncpy(pval, val, MaxVal);
  pval[MaxVal-1] = '\0';
  bool r = parseValue(pval, false);
  if (disabled(StreamOutput))
    return;
  if (r) {
    valueStr(pval);
    stream.printf("%*sset %-25s to %s\n",
		  indentation(), "", keyname, pval);
  }
  else
    stream.printf("%*s%s is not a valid value for %s\n",
		  indentation(), "", val, keyname);
}


int Parameter::put(int addr, Stream &stream) const {
  if (disabled(EEPROMPut) || name() == 0 || strlen(name()) == 0)
    return addr;
  // write first and middle character of name to EEPROM:
  size_t i = strlen(name());
  i /= 2;
  if (i == 0)
    i = 1;
  if (i >= strlen(name()))
    i = strlen(name()) - 1;
  EEPROM.update(addr++, name()[0]);
  EEPROM.update(addr++, name()[i]);
  EEPROM.update(addr++, name()[strlen(name() - 1)]);
  // write value:
  int addr1 = putValue(addr);
  char s[MaxVal];
  valueStr(s);
  stream.printf("Wrote %s with value \"%s\" ", name(), s);
  stream.printf("to EEPROM at address %04x\n", addr);
  return addr1;
}


int Parameter::get(int addr, bool setvalue, Stream &stream) {
  if (disabled(EEPROMGet) || name() == 0 || strlen(name()) == 0)
    return addr;
  // check first and middle character of name in EEPROM:
  char c0 = EEPROM.read(addr++);
  char c1 = EEPROM.read(addr++);
  char c2 = EEPROM.read(addr++);
  size_t i = strlen(name());
  i /= 2;
  if (i == 0)
    i = 1;
  if (i >= strlen(name()))
    i = strlen(name()) - 1;
  if (c0 != name()[0] || c1 != name()[i] || c2 != name()[strlen(name()) - 1])
    return -1;
  // read value:
  int addr1 = getValue(addr, setvalue);
  if (setvalue) {
    char s[MaxVal];
    valueStr(s);
    stream.printf("Read \"%s\" for %s ", s, name());
    stream.printf("from EEPROM at address %04x\n", addr);
  }
  return addr1;
}


void Parameter::instructions(char *str) const {
  *str = '\0';
  if (detailed())
    strcpy(str, TypeStr);
}


char Parameter::UnitPref[NUnits][6] = {
  "Deka", "deka", "Hekto", "hekto", "kilo", "Kilo", 
  "Mega", "mega", "Giga", "giga", "Tera", "tera", 
  "Peta", "peta", "Exa", "exa", 
  "Dezi", "dezi", "Zenti", "centi", "Micro", "micro", 
  "Milli", "milli", "Nano", "nano", "Piko", "piko", 
  "Femto", "femto", "Atto", "atto", 
  "da", "h", "K", "k", "M", "G", "T", "P", "E", 
  "d", "c", "mu", "u", "m", "n", "p", "f", "a"
};


float Parameter::UnitFac[NUnits] = {
  1e1,  1e1,  1e2,  1e2,  1e3,  1e3,  
  1e6,  1e6,  1e9,  1e9,  1e12, 1e12, 
  1e15, 1e15, 1e18, 1e18,
  1e-1, 1e-1, 1e-2, 1e-2, 1e-6, 1e-6, 
  1e-3, 1e-3, 1e-9, 1e-9, 1e-12, 1e-12, 
  1e-15, 1e-15, 1e-18, 1e-18, 
  1e1,  1e2,  1e3,  1e3,  1e6,  1e9, 1e12, 1e15, 1e18,
  1e-1, 1e-2, 1e-6, 1e-6, 1e-3, 1e-9, 1e-12, 1e-15, 1e-18
};


float Parameter::changeUnit(float val, const char *oldunit,
			    const char *newunit) {
  // adapted from https://github.com/relacs/relacs/blob/1facade622a80e9f51dbf8e6f8171ac74c27f100/options/src/parameter.cc#L1647-L1703

  // missing unit?
  if (newunit == 0 || strlen(newunit) == 0 ||
      oldunit == 0 || strlen(oldunit) == 0)
    return val;

  // parse old unit:
  float f1 = 1.0;
  if (strcmp(oldunit, "%") == 0)
    f1 = 0.01;
  else if (strcmp(oldunit, "hour") == 0 || strcmp(oldunit, "h") == 0)
    f1 = 60.0*60.0;
  else if (strcmp(oldunit, "min") == 0)
    f1 = 60.0;
  else {
    int k = 0;
    for (k=0; k<NUnits; k++)
      if (strncmp(oldunit, UnitPref[k], strlen(UnitPref[k])) == 0)
	break;
    if (k < NUnits && strlen(UnitPref[k]) < strlen(oldunit))
      f1 = UnitFac[k];
  }
  
  // parse new unit:
  float f2 = 1.0;
  if (strcmp(newunit, "%") == 0)
    f2 = 0.01;
  else if (strcmp(newunit, "hour") == 0 || strcmp(newunit, "h") == 0)
    f2 = 60.0*60.0;
  else if (strcmp(newunit, "min") == 0)
    f2 = 60.0;
  else {
    int k = 0;
    for (k=0; k<NUnits; k++)
      if (strncmp(newunit, UnitPref[k], strlen(UnitPref[k])) == 0)
	break;
    if (k < NUnits && strlen(UnitPref[k]) < strlen(newunit))
      f2 = UnitFac[k];
  }
  
  return val * f1/f2;
}


BaseStringParameter::BaseStringParameter(Menu &menu, const char *name) :
  Parameter(menu, name),
  Selection(0) {
}


BaseStringParameter::BaseStringParameter(Menu &menu,
					 const char *name,
					 const char **selection,
					 size_t n) :
  Parameter(menu, name, n),
  Selection(selection) {
}


void BaseStringParameter::setSelection(const char **selection, size_t n) {
  NSelection = n;
  Selection = selection;
}


int BaseStringParameter::checkSelection(const char *val) {
  if (NSelection == 0)
    return 0;
  for (size_t k=0; k<NSelection; k++)
    if (strcmp(Selection[k], val) == 0)
      return k;
  return -1;
}


void BaseStringParameter::listSelection(Stream &stream) const {
  for (size_t k=0; k<NSelection; k++)
    stream.printf("  - %d) %s\n", k+1, Selection[k]);
}


const char *BaseStringParameter::YesNoStrings[2] = {"no", "yes"};
const bool BaseStringParameter::BoolEnums[2] = {false, true};


ConstStringParameter::ConstStringParameter(Menu &menu, const char *name,
					   const char *str) :
  BaseStringParameter(menu, name),
  Value(str) {
  sprintf(TypeStr, "string %hu", strlen(str) + 1);
  disableSupported(SetValue);
}


bool ConstStringParameter::setValue(const char *val) {
  if (val == 0)
    return false;
  Value = val;
  return true;
}


void ConstStringParameter::valueStr(char *str) const {
  strncpy(str, Value, MaxVal);
  str[MaxVal - 1] = '\0';
}


BoolParameter::BoolParameter(Menu &menu, const char *name, bool val) :
  EnumParameter<bool>(menu, name, val, BoolEnums, YesNoStrings, 2) {
  strcpy(TypeStr, "boolean");
}


BoolPointerParameter::BoolPointerParameter(Menu &menu,
					   const char *name, bool *val) :
  EnumPointerParameter<bool>(menu, name, val, BoolEnums, YesNoStrings, 2) {
  strcpy(TypeStr, "boolean");
}


