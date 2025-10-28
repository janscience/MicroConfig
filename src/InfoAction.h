/*
  InfoAction - Action printing out key-value pairs.
  Created by Jan Benda, October 23rd, 2025.
*/

#ifndef InfoAction_h
#define InfoAction_h


#include <Action.h>


class InfoAction : public Action {

 public:

  /* Initialize and add to configuration menu.
     You need to add ket-value pairs later on using add(). */
  InfoAction(Menu &menu, const char *name, int roles=StreamIO | Report);

  /* Initialize and add to configuration menu.
     Key-value pairs are added to the action using add().
     The InfoAction gets StreamIO and Report roles enabled. */
  InfoAction(Menu &menu, const char *name, const char *key1, const char *value1,
	     const char *key2=0, const char *value2=0, const char *key3=0, const char *value3=0,
	     const char *key4=0, const char *value4=0, const char *key5=0, const char *value5=0);

  /* Add a key-value pair.
     The strings are not copied, only pointers are stored.
     Make sure to pass in static strings.
     Return the index of the added key-value pair.
     If nothing was added, return -1. */
  int add(const char *key, const char *value);

  /* Set value of key-vailue pair at index. */
  void setValue(size_t index, const char *value);

  /* Set value of key-vailue pair. */
  void setValue(const char *key, const char *value);

  /* Save the actions's name and all the key-value pairs to file.
     roles must be enabled. */
  virtual void save(File &file, int roles=FileOutput, size_t indent=0, size_t w=0) const;

  /* Print out text on stream. */
  virtual void execute(Stream &stream=Serial, unsigned long timeout=0,
		       bool echo=true, bool detailed=false);


 protected:

  static const size_t MaxKeyVals = 16;
  const char *Keys[MaxKeyVals];
  const char *Values[MaxKeyVals];
  size_t NKeyVals;
  size_t MaxWidth;
  
};


#endif
