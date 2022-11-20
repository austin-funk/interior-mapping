#version 330 core

//in vec3 col;
//out vec4 fragColor;

//void main() {
//   fragColor = vec4(col, 1.0);
//}

// Task 5: declare "in" variables for the world-space position and normal,
//         received post-interpolation from the vertex shader
in vec3 world_pos;
in vec3 world_norm;

// Task 10: declare an out vec4 for your output color
out vec4 fragColor;

struct light {
    // 0 = directional, 1 = point, 2 = spotlight
    int type; // make this smallers?
    vec4 pos;
    vec4 dir;
    vec4 color;
};

uniform light lights[8];
uniform int numLights;

uniform vec4 material_ambO;
uniform vec4 material_difO;
uniform vec4 material_specO;

// Task 12: declare relevant uniform(s) here, for ambient lighting
uniform float k_a; // this is a gross way to do this!

// Task 13: declare relevant uniform(s) here, for diffuse lighting
uniform float k_d;
//uniform vec4 light_pos[8]; // change code to work with 8 lights,
////                            light should be a struct containing type, pos, dir

// Task 14: declare relevant uniform(s) here, for specular lighting
uniform float k_s;
uniform float shiny;
uniform vec4 camera_pos;

void main() {
    // Remember that you need to renormalize vectors here if you want them to be normalized

    // Task 10: set your output color to white (i.e. vec4(1.0)). Make sure you get a white circle!
    // fragColor = vec4(1.0);

    // Task 11: set your output color to the absolute value of your world-space normals,
    //          to make sure your normals are correct.
    // fragColor = vec4(abs(world_norm), 1.0);
//    fragColor = vec4(world_norm, 1);
//    return;
    vec4 illum = vec4(0, 0, 0, 1);
    vec4 addedIllum = vec4(0, 0, 0, 1);

    vec3 L;
    float fatt;
    //float dist; // needed for other light types
    float spotlightFactor;

    for (int i = 0; i < numLights; ++i) {
        spotlightFactor = 1.0f;
        if (lights[i].type == 0) {
            L = normalize(vec3(-lights[i].dir));
            fatt = 1.0f;
        } else {
            continue;
        }
        //dist = length(vec3(lights[i].pos) - world_pos);

        // Ambient term
        addedIllum += (k_a * material_ambO);

        // diffuse term
        float NdotL = dot(world_norm, L);
        vec4 trueColor = k_d * material_difO;
        NdotL = (NdotL < 0) ? 0 : NdotL;
        NdotL = (NdotL > 1) ? 1 : NdotL;
        addedIllum +=  (NdotL >= 0) ? (fatt * lights[i].color * trueColor * NdotL) : vec4(0, 0, 0, 0);



        /*
        glm::vec3 R = glm::reflect(L, normal);
        float RdotDirCam = glm::dot(R, glm::vec3(worldRay.d));
        addedIllum += (RdotDirCam > 0) ?
                    Fatt * light.color *
                    (globalData->ks * material.cSpecular * powf(RdotDirCam, material.shininess)):
                    glm::vec4(0.0f);
        */



        // specular term
        vec3 R = reflect(L, world_norm);
        float RdotDirCam = dot(R, normalize(world_pos - vec3(camera_pos)));//dot(R, vec3(camera_pos) - world_pos); // check this
        RdotDirCam = (RdotDirCam < 0.0f) ? 0.0f : RdotDirCam;
        RdotDirCam = (RdotDirCam > 1.0f) ? 1.0f : RdotDirCam;
        float shinyPower = (RdotDirCam == 0.0f && shiny == 0.0f) ? 0.0f : pow(RdotDirCam, shiny);
        addedIllum += (fatt * lights[i].color * k_s * shinyPower * material_specO);

        illum += (addedIllum * spotlightFactor);
        addedIllum = vec4(0, 0, 0, 1);
    }

    // Task 12: add ambient component to output color
//    color = k_a;

//    // Task 13: add diffuse component to output color
//    vec4 L = normalize(light_pos - vec4(world_pos, 1.0f));
//    float lightRefl = dot(vec4(normalize(world_norm), 0.0f), L);
//    lightRefl = (lightRefl < 0) ? 0 : lightRefl;
//    lightRefl = (lightRefl > 1) ? 1 : lightRefl;
//    float difColor = k_d * lightRefl;
//    color += difColor;

//    // Task 14: add specular component to output color
//    vec4 R = normalize(reflect(L, vec4(world_norm, 0.0f)));
//    vec4 camDir = normalize(vec4(world_pos, 1.0f) - camera_pos);
//    float dotRE = dot(R, camDir);
//    dotRE = (dotRE < 0) ? 0 : dotRE;
//    dotRE = (dotRE > 1) ? 1 : dotRE;
//    float specColor = k_s * pow(dotRE, shiny);
//    color += specColor;

//    std::uint8_t r = 255 * fmin(fmax(color.r, 0), 1);
//    std::uint8_t g = 255 * fmin(fmax(color.g, 0), 1);
//    std::uint8_t b = 255 * fmin(fmax(color.b, 0), 1);

    fragColor = //illum;
                vec4(min(max(illum[0], 0), 1),
                     min(max(illum[1], 0), 1),
                     min(max(illum[2], 0), 1),
                     min(max(illum[3], 0), 1));
}
