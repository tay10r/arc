layout (location = 0) in vec2 position;

out highp vec2 fragCoord;

void
main()
{
  fragCoord = position;
  gl_Position = vec4(position, 0.0, 1.0);
}
