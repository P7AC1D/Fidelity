#pragma once
#include <vector>
#include "../Query.hpp"

/**
 * GLQueryPool
 * ---------------------------------
 * OpenGL implementation of IQueryPool.
 *
 * Responsibilities:
 * - Allocate and delete GL query object IDs for the requested count.
 * - Expose pool validity and native handle for backend usage.
 * - Provide accessors to individual GL query IDs.
 *
 * Notes:
 * - Supports QueryType::Timestamp, QueryType::TimeElapsed, and QueryType::SamplesPassed.
 * - Uses core OpenGL queries available in GL 3.3+/4.1 (no newer APIs required).
 */
class GLQueryPool final : public IQueryPool
{
public:
  explicit GLQueryPool(const QueryPoolDesc &desc);
  ~GLQueryPool();

  // ResourceHandle
  void *getNativeHandle() const override { return (void *)_queries.data(); }
  bool isValid() const override { return !_queries.empty(); }

  // GL-specific helpers
  unsigned int getId(uint32 index) const { return _queries.at(index); }
  QueryType getType() const { return _desc.type; }

private:
  std::vector<unsigned int> _queries;
};
