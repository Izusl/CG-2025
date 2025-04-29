#ifdef _WIN32
#include <windows.h>
#include <SDL_main.h>
#endif

#include <glm/glm.hpp>
#include <string>
#include <fstream>
#include "Canis/Canis.hpp"
#include "Canis/Entity.hpp"
#include "Canis/Graphics.hpp"
#include "Canis/Window.hpp"
#include "Canis/Shader.hpp"
#include "Canis/Debug.hpp"
#include "Canis/IOManager.hpp"
#include "Canis/InputManager.hpp"
#include "Canis/Camera.hpp"
#include "Canis/Model.hpp"
#include "Canis/World.hpp"
#include "Canis/Editor.hpp"
#include "Canis/FrameRateManager.hpp"

using namespace glm;

// git restore .
// git fetch
// git pull

// 3d array
std::vector<std::vector<std::vector<unsigned int>>> map = {};

// declaring functions
void SpawnLights(Canis::World &_world);
void LoadMap(std::string _path);

#ifdef _WIN32
#define main SDL_main
extern "C" int main(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    Canis::Init();
    Canis::InputManager inputManager;
    Canis::FrameRateManager frameRateManager;
    frameRateManager.Init(60);

    /// SETUP WINDOW
    Canis::Window window;
    window.MouseLock(true);

    unsigned int flags = 0;

    if (Canis::GetConfig().fullscreen)
        flags |= Canis::WindowFlags::FULLSCREEN;

    window.Create("Hello Graphics", Canis::GetConfig().width, Canis::GetConfig().heigth, flags);
    /// END OF WINDOW SETUP

    Canis::World world(&window, &inputManager, "assets/textures/lowpoly-skybox/");
    SpawnLights(world);

    Canis::Editor editor(&window, &world, &inputManager);

    Canis::Graphics::EnableAlphaChannel();
    Canis::Graphics::EnableDepthTest();

    /// SETUP SHADER
    Canis::Shader shader;
    shader.Compile("assets/shaders/hello_shader.vs", "assets/shaders/hello_shader.fs");
    shader.AddAttribute("aPosition");
    shader.Link();
    shader.Use();
    shader.SetInt("MATERIAL.diffuse", 0);
    shader.SetInt("MATERIAL.specular", 1);
    shader.SetFloat("MATERIAL.shininess", 64);
    shader.SetBool("WIND", false);
    shader.UnUse();

    Canis::Shader grassShader;
    grassShader.Compile("assets/shaders/hello_shader.vs", "assets/shaders/hello_shader.fs");
    grassShader.AddAttribute("aPosition");
    grassShader.Link();
    grassShader.Use();
    grassShader.SetInt("MATERIAL.diffuse", 0);
    grassShader.SetInt("MATERIAL.specular", 1);
    grassShader.SetFloat("MATERIAL.shininess", 64);
    grassShader.SetBool("WIND", true);
    grassShader.SetFloat("WINDEFFECT", 0.2);
    grassShader.UnUse();
    /// END OF SHADER

    /// Load Image
    Canis::GLTexture dirt = Canis::LoadImageGL("assets/textures/dirt.png", true);

    Canis::GLTexture grass = Canis::LoadImageGL("assets/textures/grass.png", false);
    Canis::GLTexture poppy = Canis::LoadImageGL("assets/textures/poppy.png", false);
    Canis::GLTexture blue_orchid = Canis::LoadImageGL("assets/textures/blue_orchid.png", false);
    Canis::GLTexture allium = Canis::LoadImageGL("assets/textures/allium.png", false);
    Canis::GLTexture white_tulip = Canis::LoadImageGL("assets/textures/white_tulip.png", false);
    Canis::GLTexture lily_of_the_valley = Canis::LoadImageGL("assets/textures/lily_of_the_valley.png", false);
    Canis::GLTexture azure_bluet = Canis::LoadImageGL("assets/textures/azure_bluet.png", false);

    Canis::GLTexture oak_log = Canis::LoadImageGL("assets/textures/oak_log.png", true);
    Canis::GLTexture cobblestone = Canis::LoadImageGL("assets/textures/cobblestone.png", true);
    Canis::GLTexture white_wool = Canis::LoadImageGL("assets/textures/white_wool.png", true);

    Canis::GLTexture textureSpecular = Canis::LoadImageGL("assets/textures/container2_specular.png", true);
    /// End of Image Loading

    /// Load Models
    Canis::Model cubeModel = Canis::LoadModel("assets/models/cube.obj");
    Canis::Model grassModel = Canis::LoadModel("assets/models/plants.obj");
    /// END OF LOADING MODEL

    // Load Map into 3d array
    LoadMap("assets/maps/level.map");

    // Loop map and spawn objects
    for (int y = 0; y < map.size(); y++)
    {
        for (int x = 0; x < map[y].size(); x++)
        {
            for (int z = 0; z < map[y][x].size(); z++)
            {
                Canis::Entity entity;
                entity.active = true;

                switch (map[y][x][z])
                {
                    case 0:
                        break; // air


                    case 1: //dirt
                        entity.tag = "dirt";
                        entity.albedo = &dirt;
                        entity.specular = &textureSpecular;
                        entity.model = &cubeModel;
                        entity.shader = &shader;
                        entity.transform.position = vec3(x + 0.0f, y + 0.0f, z + 0.0f);
                        world.Spawn(entity);
                        break;


                    case 2: { // short grass or flower
                        if (std::rand() % 2 != 0) { 
                            // 50% chance to place something, adjust for short grass 
                            int grassChance = std::rand() % 100;
                            
                            if (grassChance < 90) { // 90% chance to place short grass
                                entity.tag = "short_grass"; // short grass tag
                                entity.albedo = &grass; // short grass texture
                                entity.specular = &textureSpecular;
                                entity.model = &grassModel;
                                entity.shader = &grassShader;
                                entity.transform.position = vec3(x, y, z);
                                world.Spawn(entity);
                                break;
                            }
                        }
                    
                        // 30% chance for flowers if not placed short grass
                        static const std::vector<std::pair<const char*, Canis::GLTexture*>> floraOptions = {
                            {"poppy", &poppy},
                            {"blue_orchid", &blue_orchid},
                            {"allium", &allium},
                            {"white_tulip", &white_tulip},
                            {"lily_of_the_valley", &lily_of_the_valley},
                            {"azure_bluet", &azure_bluet}
                        };
                    
                        int index = std::rand() % floraOptions.size();
                        entity.tag = floraOptions[index].first;
                        entity.albedo = floraOptions[index].second;
                        entity.specular = &textureSpecular;
                        entity.model = &grassModel;
                        entity.shader = &grassShader;
                        entity.transform.position = vec3(x, y, z);
                        world.Spawn(entity);
                        break;
                    }


                    case 3: // oaklog
                        entity.tag = "oak_log";
                        entity.albedo = &oak_log;
                        entity.specular = &textureSpecular;
                        entity.model = &cubeModel;
                        entity.shader = &shader;
                        entity.transform.position = vec3(x + 0.0f, y + 0.0f, z + 0.0f);
                        world.Spawn(entity);
                        break;


                    case 4: // cobblestone
                        entity.tag = "cobblestone";
                        entity.albedo = &cobblestone;
                        entity.specular = &textureSpecular;
                        entity.model = &cubeModel;
                        entity.shader = &shader;
                        entity.transform.position = vec3(x + 0.0f, y + 0.0f, z + 0.0f);
                        world.Spawn(entity);
                        break;

                    case 5: // white wool
                        entity.tag = "white_wool";
                        entity.albedo = &white_wool;
                        entity.specular = &textureSpecular;
                        entity.model = &cubeModel;
                        entity.shader = &shader;
                        entity.transform.position = vec3(x + 0.0f, y + 0.0f, z + 0.0f);
                        world.Spawn(entity);
                        break;


                    default:
                        break;
                }
            }
        }
    }

    double deltaTime = 0.0;
    double fps = 0.0;

    // Application loop
    while (inputManager.Update(Canis::GetConfig().width, Canis::GetConfig().heigth))
    {
        deltaTime = frameRateManager.StartFrame();
        Canis::Graphics::ClearBuffer(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);

        world.Update(deltaTime);
        world.Draw(deltaTime);

        editor.Draw();

        window.SwapBuffer();

        // EndFrame will pause the app when running faster than frame limit
        fps = frameRateManager.EndFrame();

        Canis::Log("FPS: " + std::to_string(fps) + " DeltaTime: " + std::to_string(deltaTime));
    }

    return 0;
}

void LoadMap(std::string _path)
{
    std::ifstream file;
    file.open(_path);

    if (!file.is_open())
    {
        printf("file not found at: %s \n", _path.c_str());
        exit(1);
    }

    int number = 0;
    int layer = 0;

    map.push_back(std::vector<std::vector<unsigned int>>());
    map[layer].push_back(std::vector<unsigned int>());

    while (file >> number)
    {
        if (number == -2) // add new layer
        {
            layer++;
            map.push_back(std::vector<std::vector<unsigned int>>());
            map[map.size() - 1].push_back(std::vector<unsigned int>());
            continue;
        }

        if (number == -1) // add new row
        {
            map[map.size() - 1].push_back(std::vector<unsigned int>());
            continue;
        }

        map[map.size() - 1][map[map.size() - 1].size() - 1].push_back((unsigned int)number);
    }
}

void SpawnLights(Canis::World &_world)
{
    Canis::DirectionalLight directionalLight;
    _world.SpawnDirectionalLight(directionalLight);

    Canis::PointLight pointLight;
    pointLight.position = vec3(0.0f);
    pointLight.ambient = vec3(0.2f);
    pointLight.diffuse = vec3(0.5f);
    pointLight.specular = vec3(1.0f);
    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;

    _world.SpawnPointLight(pointLight);

    pointLight.position = vec3(0.0f, 0.0f, 1.0f);
    pointLight.ambient = vec3(0.0f, 0.0f, 0.0f);

    _world.SpawnPointLight(pointLight);

    pointLight.position = vec3(-2.0f);
    pointLight.ambient = vec3(0.0f, 0.0f, 0.0f);

    _world.SpawnPointLight(pointLight);

    pointLight.position = vec3(2.0f);
    pointLight.ambient = vec3(0.0f, 0.0f, 0.0f);

    _world.SpawnPointLight(pointLight);
}