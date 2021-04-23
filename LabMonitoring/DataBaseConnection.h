#ifndef DATABASECON
#define DATABASECON


#include "SensorBoard.h"

class DataBaseConnection {
public:
  virtual bool writeToDataBase(SensorBoard &SensorBoard_obj, bool arduino_just_resetted, bool rtc_did_not_work_send_data_to_late, bool send_additional_debug_data) { return false; };
};

#endif
