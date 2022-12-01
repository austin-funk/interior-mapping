#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

/*
layout (location = 0) in vec3 pos;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

out vec4 world_pos;
out vec4 world_normal;
out vec4 camera_pos;

void main() {
   gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(pos,1);
   world_pos = modelMatrix * vec4(pos,1);
   world_normal = transpose(inverse(modelMatrix)) * normalize(vec4(pos,0));
   camera_pos = inverse(viewMatrix) * vec4(0,0,0,1);
}
  */


out vec3 world_pos;
out vec3 world_norm;

// model and M for normal
uniform mat4 model_mat;
uniform mat3 inv_tran_model_mat;

// view and projection
uniform mat4 view_mat;
uniform mat4 projection_mat;

void main() {
    // define pos and norm for fragment shader
    world_pos = vec3(model_mat * vec4(position, 1));
    world_norm = normalize(inv_tran_model_mat * normal);

    // set position so that openGL can clip
    gl_Position = projection_mat * view_mat * vec4(world_pos, 1);
}
