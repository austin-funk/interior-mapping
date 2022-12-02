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
    vec3 point_coeffs;
    float angle;
    float penumbra;
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

// post-processing
uniform bool invert;

void main() {
    vec4 illum = vec4(0, 0, 0, 1);
    vec4 addedIllum = vec4(0, 0, 0, 1);

    vec3 L;
    float fatt;
    float spotlightFactor;
    float dist;

    bool brk = false;

    vec3 w_norm = normalize(world_norm);

    for (int i = 0; i < numLights; ++i) {
        // spotlight factor
        spotlightFactor = 1.0f;

        switch (lights[i].type) {
        case 0: // directional light
            L = normalize(vec3(-lights[i].dir));
            fatt = 1.0f;
            break;
        case 1: // point light
            fatt = min(1.0f,
                       1.0f / (lights[i].point_coeffs.x + (dist * lights[i].point_coeffs.y) + (pow(dist, 2) * lights[i].point_coeffs.z)));
            L = normalize(vec3(lights[i].pos) - world_pos);
            break;
        case 2: // spot light
            dist = length(vec3(lights[i].pos) - world_pos);
            fatt = min(1.0f,
                       1.0f / (lights[i].point_coeffs.x + (dist * lights[i].point_coeffs.y) + (pow(dist, 2) * lights[i].point_coeffs.z)));
            L = normalize(vec3(lights[i].pos) - world_pos);

            float x = acos(dot(L, vec3(-normalize(lights[i].dir))));
            float thetaInner = lights[i].angle - lights[i].penumbra;
            if (x > lights[i].angle) {
                spotlightFactor = 0;
            } else if (x > thetaInner) {
                float angleDif = (x - thetaInner) / (lights[i].penumbra);
                float falloff = (-2 * pow(angleDif, 3)) + (3 * pow(angleDif, 2));
                spotlightFactor = 1 - falloff;
            }
            break;
        default: // not a light
            brk = true;
            break;
        }

        if (brk) {
            brk = false;
            continue;
        }

        // Ambient term
        addedIllum += (k_a * material_ambO);

        // diffuse term
        float NdotL = clamp(dot(w_norm, L), 0, 1);
        vec4 trueColor = k_d * material_difO;
        //NdotL = (NdotL > 1) ? 1 : NdotL;
        addedIllum +=  (NdotL >= 0) ? (fatt * lights[i].color * trueColor * NdotL) : vec4(0, 0, 0, 0);

        // specular term
        vec3 R = reflect(-L, w_norm);
        float RdotDirCam = clamp(dot(normalize(vec3(camera_pos) - world_pos), R), 0, 1);//camera_pos
        // RdotDirCam = (RdotDirCam < 0.0f) ? 0.0f : RdotDirCam;
        // RdotDirCam = (RdotDirCam > 1.0f) ? 1.0f : RdotDirCam;
        float shinyPower = (RdotDirCam == 0.0f) ? 0.0f : pow(RdotDirCam, shiny);// && shiny == 0.0f
        addedIllum += (fatt * lights[i].color * k_s * shinyPower * material_specO);

        illum += (addedIllum * spotlightFactor);
        addedIllum = vec4(0, 0, 0, 1);
    }

    float r = min(max(illum[0], 0), 1),
          g = min(max(illum[1], 0), 1),
          b = min(max(illum[2], 0), 1),
          a = min(max(illum[3], 0), 1);

    fragColor = vec4(r, g, b, a);
}
