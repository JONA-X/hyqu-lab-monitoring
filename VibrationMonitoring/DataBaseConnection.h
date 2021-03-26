#ifndef DATABASECON
#define DATABASECON


class DataBaseConnection {
public:
  virtual bool writeToDataBase(double MaxAcc, double AvgAcc) { return false; };
};

#endif
