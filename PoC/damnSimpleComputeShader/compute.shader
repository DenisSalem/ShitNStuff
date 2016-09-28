#version 430

layout (local_size_x = 1, local_size_y = 1) in;
layout (rgba32f, binding = 0) uniform image2D img_output;

void main() {
  // Aucune donnée n'étant passé au moment de la création de la texture,
  // on met la couleur par défaut en orange.
  vec4 pixel = vec4(vec3(1.0,0.5,0.0), 1.0);
  ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);

  // shit n stuff here

  imageStore(img_output, pixel_coords, pixel);
}
