#version 330 core

in vec3 world_pos;
in vec3 world_norm;

out vec4 fragColor;

// light struct
struct light {
    // 0 = directional, 1 = point, 2 = spotlight
    int type;
    vec4 pos;
    vec4 dir;
    vec4 color;
};

// light data
uniform light lights[8];
uniform int numLights;

// material data
uniform vec4 material_ambO;
uniform vec4 material_difO;
uniform vec4 material_specO;

// phong data
uniform float k_a;
uniform float k_d;
uniform float k_s;
uniform float shiny;

// camera data
uniform vec4 camera_pos;

void main() {
    vec4 illum = vec4(0, 0, 0, 1);
    vec4 addedIllum = vec4(0, 0, 0, 1);

    vec3 L;
    float fatt;
    float spotlightFactor;

    for (int i = 0; i < numLights; ++i) {
        spotlightFactor = 1.0f;
        if (lights[i].type == 0) {
            L = normalize(vec3(-lights[i].dir));
            fatt = 1.0f;
        } else {
            continue;
        }

        // Ambient term
        addedIllum += (k_a * material_ambO);

        // diffuse term
        float NdotL = dot(normalize(world_norm), L);
        vec4 trueColor = k_d * material_difO;
        NdotL = (NdotL < 0) ? 0 : NdotL;
        NdotL = (NdotL > 1) ? 1 : NdotL;
        addedIllum +=  (NdotL >= 0) ? (fatt * lights[i].color * trueColor * NdotL) : vec4(0, 0, 0, 0);

        // specular term
        vec3 R = reflect(L, normalize(world_norm));
        float RdotDirCam = dot(R, normalize(world_pos - vec3(camera_pos)));
        RdotDirCam = (RdotDirCam < 0.0f) ? 0.0f : RdotDirCam;
        RdotDirCam = (RdotDirCam > 1.0f) ? 1.0f : RdotDirCam;
        float shinyPower = (RdotDirCam == 0.0f && shiny == 0.0f) ? 0.0f : pow(RdotDirCam, shiny);
        addedIllum += (fatt * lights[i].color * k_s * shinyPower * material_specO);

        illum += (addedIllum * spotlightFactor);
        addedIllum = vec4(0, 0, 0, 1);
    }

    fragColor = vec4(min(max(illum[0], 0), 1),
                     min(max(illum[1], 0), 1),
                     min(max(illum[2], 0), 1),
                     min(max(illum[3], 0), 1));
}
