#ifndef DATABASECON
#define DATABASECON


#include "DataObject.h"

class DataBaseConnection {
public:
  virtual bool writeToDataBase(DataObject &Data, bool arduino_just_resetted, bool rtc_did_not_work_send_data_to_late) { return false; };
};

#endif
