#include "Renderer.hpp"

#include <iostream>

void Renderer::Draw(const VertexArray& va, const IndexBuffer& ib) const {
    va.Bind();
    ib.Bind();
    GLCall(glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr));
}

void Renderer::Clear() const {
    GLCall(glClearColor(0.1137f, 0.1255f, 0.1725f, 1.0f));
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
}