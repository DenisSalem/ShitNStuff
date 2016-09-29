#version 430

layout (local_size_x = 1, local_size_y = 1) in;
layout (rgba32f, binding = 0) uniform image2D img_output;

void main() {
  // Aucune donnée n'étant passé au moment de la création de la texture,
  // on la remplit d'une couleur par défaut, en orange.
  
  ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
  vec4 pixel = vec4(1.0,.5,.0, 1.0);

  imageStore(img_output, coords, pixel);
}
