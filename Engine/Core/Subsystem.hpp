#pragma once

template <typename T>
class Subsystem
{
public:
  static T* Instance();
  
  virtual ~Subsystem();
  
protected:
  Subsystem();
  
private:
  static T* _instance;
};
