#include "GLQueryPool.hpp"
#include "GL.hpp"
#include "GLCommon.hpp"

GLQueryPool::GLQueryPool(const QueryPoolDesc &desc)
    : IQueryPool(desc)
{
  _queries.resize(desc.count, 0u);
  glCall(glGenQueries(static_cast<GLsizei>(desc.count), reinterpret_cast<GLuint *>(_queries.data())));
}

GLQueryPool::~GLQueryPool()
{
  if (!_queries.empty())
  {
    glCall(glDeleteQueries(static_cast<GLsizei>(_queries.size()), reinterpret_cast<const GLuint *>(_queries.data())));
  }
}
