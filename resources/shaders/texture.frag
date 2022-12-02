#version 330 core

// Task 16: Create a UV coordinate in variable
in vec2 uv_coord;

// Task 8: Add a sampler2D uniform
uniform sampler2D kitten;

// Task 29: Add a bool on whether or not to filter the texture
uniform bool pixel_processing;
uniform bool filter_processing;

uniform float width;
uniform float height;

out vec4 fragColor;

void main()
{
    //fragColor = vec4(1);
    // Task 17: Set fragColor using the sampler2D at the UV coordinate
    fragColor = texture(kitten, uv_coord);

    if (filter_processing) {
        vec2 above = vec2(uv_coord.x, ((uv_coord.y * height) + 1) / height);
        vec2 below = vec2(uv_coord.x, ((uv_coord.y * height) - 1) / height);
        vec2 right = vec2(uv_coord.x, ((uv_coord.y * width) + 1) / width);
        vec2 left = vec2(uv_coord.x, ((uv_coord.y * width) - 1) / width);
        fragColor = 5 * texture(kitten, uv_coord) - texture(kitten, above)
                    - texture(kitten, below) - texture(kitten, right)
                    - texture(kitten, left);
    }

    // Task 33: Invert fragColor's r, g, and b color channels if your bool is true
    if (pixel_processing) {
        fragColor = vec4(1.0f - fragColor[0], 1.0f - fragColor[1], 1.0f - fragColor[2], fragColor[3]);
    }
}
