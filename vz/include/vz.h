/**
 * @file vz.h
 *
 * @brief This is the main API for the visualization library.
 *
 * @details The visualization library is mostly just a lightweight wrapper around OpenGL ES 3.
 *          Start by creating a window and context with @ref setup (use @ref teardown upon exit)
 *          and then begin and complete with frame with @ref beginFrame and @ref endFrame.
 *          Between each @ref beginFrame and @ref endFrame call, you can render with OpenGL ES 3
 *          and draw UI windows with ImGui.
 * */

#pragma once

#include <iosfwd>
#include <memory>
#include <sstream>
#include <vector>

#include <cstddef>
#include <cstdint>

#include <GLES3/gl3.h>

namespace vz {

/**
 * @brief Used for invoking user-defined shader programs.
 */
class Shader
{
public:
  enum class LogKind
  {
    VertCompile,
    FragCompile,
    Link
  };

  virtual ~Shader() = default;

  /**
   * @brief Defines a new macro.
   *
   * @param name The name of the macro.
   *
   * @param value The value to assign the macro.
   *
   * @note The macros get prepended to both the vertex shader and the fragment shader.
   */
  template<typename T>
  void define(const char* name, const T& value)
  {
    std::ostringstream tmpStream;
    tmpStream << "#define " << name << ' ' << value << '\n';
    defineImpl(name, tmpStream.str().c_str());
  }

  /**
   * @brief Compiles and links the shader.
   *
   * @param vertSource The vertex shader source code.
   *
   * @param fragSource The fragment shader source code.
   *
   * @return True on success, false on failure.
   */
  [[nodiscard]] virtual auto build(const char* vertSource, const char* fragSource) -> bool = 0;

  /**
   * @brief Gets the shader log.
   *
   * @details If an error occurs while building the shader, the log for that particular stage is read into a string can
   *          be accessed by this function.
   *
   * @return The shader log.
   */
  [[nodiscard]] virtual auto getLog(LogKind kind) const -> std::string = 0;

  /**
   * @brief Makes this shader the active shader in the OpenGL context.
   */
  virtual void use() = 0;

  /**
   * @brief Copies the data from all bound pointers and writes their values to the shader uniform values.
   * */
  virtual void syncUniforms() = 0;

  /**
   * @brief Binds a pointer to a 4x4 matrix in the application memory to a uniform object.
   *
   * @param name The name of the uniform.
   *
   * @param data A pointer to the uniform data. This may also be set to null in order to clear the uniform.
   *
   * @return True on success, false if the uniform values was not found.
   *
   * @note The pointer must remain valid while it is associated with this uniform.
   * */
  [[nodiscard]] virtual auto bindUniformMat4(const char* name, const float* data) -> bool = 0;

  [[nodiscard]] virtual auto bindUniformVec3(const char* name, const float* data) -> bool = 0;

  /**
   * @brief Prints the logs to the specified stream.
   */
  void dumpLogs(std::ostream& out) const;

protected:
  virtual void defineImpl(const char* name, const char* value) = 0;
};

/**
 * @brief This is a collection of vertices, used to describe a mesh, set of lines, or set of points.
 *
 * @details Vertex arrays in this library are classes that internally store both an OpenGL buffer object and an OpenGL
 * vertex array object. There are a simplification of the OpenGL vertex arrays that fit most basic use cases.
 */
class VertexArray
{
public:
  virtual ~VertexArray() = default;

  virtual void setVertexSizes(const std::vector<GLint>& sizes) = 0;

  /**
   * @brief Uploads data to the buffer object.
   *
   * @param data A pointer to the data to upload.
   *
   * @param size The number of bytes in the buffer to upload.
   */
  virtual void uploadData(const void* data, GLsizeiptr size, GLenum usage = GL_DYNAMIC_DRAW) = 0;

  template<typename T>
  void uploadData(const std::vector<T>& data)
  {
    uploadData(static_cast<const void*>(data.data()), data.size() * sizeof(data[0]));
  }

  virtual void draw(GLenum mode, GLsizei count, GLint first = 0) = 0;

  void draw(GLenum mode);

  [[nodiscard]] virtual auto bytesPerVertex() const -> GLsizeiptr = 0;

  [[nodiscard]] virtual auto totalBytes() const -> GLsizeiptr = 0;
};

/**
 * @brief Can be used for off-screen rendering.
 * */
class Framebuffer
{
public:
  struct Size final
  {
    GLint width{};

    GLint height{};
  };

  virtual ~Framebuffer() = default;

  /**
   * @brief Indicates if the creation of the framebuffer was successful or not.
   *
   * @return True if the framebuffer can be used, false otherwise.
   * */
  [[nodiscard]] virtual auto complete() const -> bool = 0;

  virtual void use() = 0;

  [[nodiscard]] virtual auto getColorAttachment(GLint index) -> GLuint = 0;

  [[nodiscard]] virtual auto size() const -> Size = 0;

  [[nodiscard]] virtual auto readPixels() const -> std::vector<std::uint8_t> = 0;
};

/**
 * @brief This class provides a means of manipulating a camera using a mouse.
 *
 * @note This class does not need the global state of the library and can be used independently.
 * */
class CameraController
{
public:
  static auto create() -> std::unique_ptr<CameraController>;

  virtual ~CameraController() = default;

  virtual void setPosition(float x, float y, float z) = 0;

  virtual void setGrabbed(bool grabbed) = 0;

  virtual void setMouseCoords(float x, float y) = 0;

  virtual void step(float timeDelta) = 0;

  /**
   * @brief Gets the combined projection and view matrix.
   *
   * @return A pointer to the matrix, which does not change for the rest of the controllers lifetime.
   * */
  [[nodiscard]] virtual auto getCombinedMat4() const -> const float* = 0;
};

/**
 * @brief Call this function at the start of the program.
 *        It will create a new window with an OpenGL context.
 *
 * @param windowTitle The title to give the window.
 */
[[nodiscard]] auto
setup(const char* windowTitle) -> bool;

/**
 * @brief Call this at the end of the program, to release all of the resources.
 */
void
teardown();

/**
 * @brief Call this at the beginning of a new frame.
 *
 * @return True if the frame can be rendered, false if the window should be closed.
 */
[[nodiscard]] auto
beginFrame() -> bool;

/**
 * @brief Completes the rendering of the frame.
 *
 * @note Only call this function if @ref beginFrame returns true.
 */
void
endFrame();

/**
 * @brief Set the position of the camera, in terms of NED coordinetes in meters.
 */
void
setCameraPosition(float x, float y, float z);

/**
 * @brief Sets the Euler angles of the camera rotation, in terms of radians.
 *
 * @note See @ref setCameraRotationOrder to set the extrinsic rotation order. The default is XYZ.
 */
void
setCameraRotation(float x, float y, float z);

/**
 * @brief Sets the rotation order of the camera.
 *
 * @param order The rotation order of the camera Euler angles.
 */
[[nodiscard]] auto
setCameraRotationOrder(const char* order) -> bool;

/**
 * @brief Sets whether or not the user can interact with the camera.
 */
void
setCameraInteractive(bool interactive);

/**
 * @brief Gets a screen quad vertex array.
 *
 * @details Screen quads are commonly used vertex arrays for screen space rendering or effects.
 *          To avoid having to create one each time they're needed, and as a matter of convenience, a single common one
 *          is provided by the global context.
 *
 * @return A pointer to the single common screen quad.
 * */
[[nodiscard]] auto
getScreenQuad() -> VertexArray*;

/**
 * @brief Renders a grid onto the current framebuffer.
 * */
void
renderGrid();

/**
 * @brief Creates a new shader program.
 *
 * @return A pointer to the shader program.
 */
auto
createShader() -> Shader*;

/**
 * @brief Destroys an existing shader program.
 *
 * @param shader A pointer to the shader to destroy.
 *
 * @note This is also done automatically when @ref teardown is called.
 */
void
destroyShader(Shader* shader);

/**
 * @brief Gets the built-in mesh shader.
 *
 * @return A pointer to the mesh shader.
 * */
[[nodiscard]] auto
getMeshShader() -> Shader*;

/**
 * @brief Creates a new vertex array.
 *
 * @return A new vertex array object.
 */
auto
createVertexArray() -> VertexArray*;

/**
 * @brief Destroys a vertex array.
 *
 * @note This is also done automatically when @ref teardown is called.
 */
void
destroyVertexArray(VertexArray* vertexArray);

/**
 * @brief Creates a new framebuffer.
 *
 * @param width The width of the framebuffer, in terms of pixels.
 *
 * @param height The height of the framebuffer, in terms of pixels.
 *
 * @param numColorAttachments The number of textures to attach for color outputs.
 *
 * @param includeDepthBuffer Whether to include a depth buffer attachment.
 * */
auto
createFramebuffer(int width, int height, int numColorAttachments, bool includeDepthBuffer = true) -> Framebuffer*;

/**
 * @brief Releases resources allocated by a framebuffer.
 *
 * @param fb The framebuffer to destroy.
 *
 * @note This is also done automatically when @ref teardown is called.
 * */
void
destroyFramebuffer(Framebuffer* fb);

} // namespace vz
