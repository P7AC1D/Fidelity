#pragma once
#include <memory>
#include "ResourceHandle.hpp"
#include "../Core/Types.hpp"

// Basic query support for timestamps and time elapsed

enum class QueryType : uint32_t
{
  Timestamp,     // GL_TIMESTAMP
  TimeElapsed,   // GL_TIME_ELAPSED
  SamplesPassed, // GL_SAMPLES_PASSED (optional)
};

struct QueryPoolDesc
{
  QueryType type;
  uint32 count = 1;
};

class IQueryPool : public ResourceHandle
{
public:
  explicit IQueryPool(const QueryPoolDesc &desc) : _desc(desc) {}
  virtual ~IQueryPool() = default;

  const QueryPoolDesc &getDesc() const { return _desc; }

  // ResourceHandle
  virtual void *getNativeHandle() const override = 0;
  virtual bool isValid() const override = 0;

protected:
  QueryPoolDesc _desc;
};
