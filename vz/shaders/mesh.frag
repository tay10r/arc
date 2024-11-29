in highp vec3 fragNormal;

in highp vec3 fragColor;

out lowp vec4 outColor;

uniform highp vec3 iLightDir;

uniform highp vec3 iLightColor;

uniform highp vec3 iAmbientColor;

void
main()
{
  highp vec3 diffuse = mix(iAmbientColor, iLightColor, max(dot(iLightDir, fragNormal), 0.0));

  outColor = vec4(fragColor * diffuse, 1.0);
}
