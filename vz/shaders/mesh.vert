layout(location = 0) in highp vec3 position;

layout(location = 1) in highp vec3 normal;

layout(location = 2) in highp vec3 color;

uniform highp mat4 iViewProjection;

out highp vec3 fragNormal;

out highp vec3 fragColor;

void
main()
{
  fragNormal = normal;
  fragColor = color;
  gl_Position = iViewProjection * vec4(position, 1.0);
}
