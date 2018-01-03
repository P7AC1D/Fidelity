#include "Subsystem.hpp"

template <typename T>
T* Subsystem<T>::_instance = nullptr;

template <typename T>
const T* Subsystem<T>::Instance()
{
  if (_instance == nullptr)
  {
    _instance = new T();
  }
  return _instance;
}

template <typename T>
Subsystem<T>::~Subsystem()
{
  delete _instance;
  _instance = nullptr;
}

template <typename T>
Subsystem<T>::Subsystem()
{
}
