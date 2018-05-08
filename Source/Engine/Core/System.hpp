#pragma once

template <class T>
class System
{
public:
  static T* Get();

  System(System&) = delete;
  System(System&&) = delete;
  System& operator=(System&) = delete;
  System& operator=(System&&) = delete;

protected:
  System() {}
};

template <typename T>
T* System<T>::Get()
{
  static T* _instance = new T;
  return _instance;
}