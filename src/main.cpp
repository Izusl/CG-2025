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

int NUM_FIRE_FRAMES = 31; // Number of frames in the fire animation

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
    shader.SetFloat("MATERIAL.shininess", 255);
    shader.SetBool("WIND", false);
    shader.UnUse();

    Canis::Shader grassShader;
    grassShader.Compile("assets/shaders/hello_shader.vs", "assets/shaders/hello_shader.fs");
    grassShader.AddAttribute("aPosition");
    grassShader.Link();
    grassShader.Use();
    grassShader.SetInt("MATERIAL.diffuse", 0);
    grassShader.SetInt("MATERIAL.specular", 1);
    grassShader.SetFloat("MATERIAL.shininess", 1);
    grassShader.SetBool("WIND", true);
    grassShader.SetFloat("WINDEFFECT", 0.2);
    grassShader.UnUse();
    /// END OF SHADER


    /*
        This is a list of all the textures I am using
        This first section is for the grass and flowers
        The second section is for the blocks and other textures
    */
    Canis::GLTexture grass = Canis::LoadImageGL("assets/textures/grass.png", false);
    Canis::GLTexture poppy = Canis::LoadImageGL("assets/textures/poppy.png", false);
    Canis::GLTexture blue_orchid = Canis::LoadImageGL("assets/textures/blue_orchid.png", false);
    Canis::GLTexture allium = Canis::LoadImageGL("assets/textures/allium.png", false);
    Canis::GLTexture white_tulip = Canis::LoadImageGL("assets/textures/white_tulip.png", false);
    Canis::GLTexture lily_of_the_valley = Canis::LoadImageGL("assets/textures/lily_of_the_valley.png", false);
    Canis::GLTexture azure_bluet = Canis::LoadImageGL("assets/textures/azure_bluet.png", false);

    Canis::GLTexture grass_top = Canis::LoadImageGL("assets/textures/grass_block_top.png", true);
    Canis::GLTexture grass_top_specular = Canis::LoadImageGL("assets/textures/grass_top_specular.png", true);
    Canis::GLTexture glass = Canis::LoadImageGL("assets/textures/glass.png", true);
    Canis::GLTexture red_wool = Canis::LoadImageGL("assets/textures/white_wool.png", true);
    Canis::GLTexture oak_log = Canis::LoadImageGL("assets/textures/oak_log.png", true);
    Canis::GLTexture cobblestone = Canis::LoadImageGL("assets/textures/cobblestone.png", true);
    Canis::GLTexture mossy_cobblestone = Canis::LoadImageGL("assets/textures/mossy_cobblestone.png", true);
    Canis::GLTexture white_wool = Canis::LoadImageGL("assets/textures/white_wool.png", true);
    Canis::GLTexture cherry_leaves = Canis::LoadImageGL("assets/textures/cherry_leaves.png", true);
    Canis::GLTexture textureSpecular = Canis::LoadImageGL("assets/textures/container2_specular.png", true);
    Canis::GLTexture netherrack = Canis::LoadImageGL("assets/textures/netherrack.png", true);
    Canis::GLTexture stone_bricks = Canis::LoadImageGL("assets/textures/bricks.png", true);

    /// Load Models
    Canis::Model cubeModel = Canis::LoadModel("assets/models/cube.obj");
    Canis::Model grassModel = Canis::LoadModel("assets/models/plants.obj");

    /*
        This is where the fire textures are loaded
        The fire animation is 31 frames long and loaded in order
        By frames its just using the rachet way of loading multiple texture pngs
    */
    std::vector<Canis::GLTexture> fireTextures;
    for (int i = 1; i <= NUM_FIRE_FRAMES; i++) {
        std::string path = "assets/textures/fire_" + std::to_string(i) + ".png";
        fireTextures.push_back(Canis::LoadImageGL(path, true));
    }

    LoadMap("assets/maps/level.map");

    /*
        This is where everything is placed from the level.map file
        It will loop through the 3d array and spawn entities based on the value in the array
        The values are as follows:
        0 = air
        1 = grass_block_top
        2 = short_grass or flower
        3 = oak_log
        4 = cobblestone or mossy_cobblestone
        5 = white_wool
        6 = red_wool
        7 = glass
        8 = cherry_leaves
        9 = netherrack
        10 = fire
    */
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
                    /*
                        Air is not spawned, so nothing is done here
                    */
                    case 0:
                        break; // air does not spawn anything

                    /*
                        Grass block. Only shows the top face of the textuew in Minecraft
                    */
                    case 1:
                        entity.tag = "grass_block_top";
                        entity.albedo = &grass_top;
                        /* 
                            Using different specutar materials by using the same texture as the albedo
                        */
                        entity.specular = &grass_top_specular;

                        entity.model = &cubeModel;
                        entity.shader = &shader; 
                        // Minecraft grass is grayscale by default so change tint
                        entity.color = vec3(0.3686, 0.6157, 0.2039);

                        entity.transform.position = vec3(x + 0.0f, y + 0.0f, z + 0.0f);
                        world.Spawn(entity);
                        break;

                    /*
                        50% chance to place short grass or flower
                        If conditions to place are met, 90% chance to place short grass, while 10% chance to place flower
                        For the flowers, there are 6 different types of flowers that can be placed which are randomly selected
                        The flowers are: poppy, blue_orchid, allium, white_tulip, lily_of_the_valley, azure_bluet

                    */
                    case 2: { // short grass or flower
                        if (std::rand() % 2 != 0) { 
                            // 50% chance to place something, adjust for short grass 
                            int grassChance = std::rand() % 100;
                            
                            if (grassChance < 90) { // 90% chance to place short grass
                                entity.tag = "short_grass"; // short grass tag
                                entity.albedo = &grass; // short grass texture
                                /* 
                                    Using different specutar materials by using the same texture as the albedo
                                */
                                entity.specular = &grass;
                                entity.model = &grassModel;
                                entity.shader = &grassShader;
                                entity.transform.position = vec3(x, y, z);
                                world.Spawn(entity);
                                break;
                            }
                        }
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
                        entity.specular = floraOptions[index].second;
                        /* 
                            Using different specutar materials by using the same texture as the albedo
                        */
                        entity.model = &grassModel;
                        entity.shader = &grassShader;
                        entity.transform.position = vec3(x, y, z);
                        world.Spawn(entity);
                        break;
                    }

                    /*
                        Oak log texture.
                    */
                    case 3: // oaklog
                        entity.tag = "oak_log";
                        entity.albedo = &oak_log;
                        entity.specular = &oak_log;
                        /* 
                            Using different specutar materials by using the same texture as the albedo
                        */
                        entity.model = &cubeModel;
                        entity.shader = &shader;
                        entity.transform.position = vec3(x, y, z);
                        world.Spawn(entity);
                        break;

                    /*
                        Cobblestone texture.
                        1 in 3 chance to place mossy cobblestone instead of cobblestone
                    */
                    case 4:
                        if (rand() % 3 == 0) {
                            entity.tag = "mossy_cobblestone";
                            entity.albedo = &mossy_cobblestone;
                        } 
                        else {
                            entity.tag = "cobblestone";
                            entity.albedo = &cobblestone;
                        }
                        entity.specular = &cobblestone;
                        /* 
                            Using different specutar materials by using the same texture as the albedo
                        */
                        entity.model = &cubeModel;
                        entity.shader = &shader;
                        entity.transform.position = vec3(x, y, z);
                        world.Spawn(entity);
                        break;

                    /*
                        White wool texture.
                    */
                    case 5:
                        entity.tag = "white_wool";
                        entity.albedo = &white_wool;
                        entity.specular = &white_wool;
                        /* 
                            Using different specutar materials by using the same texture as the albedo
                        */
                        entity.model = &cubeModel;
                        entity.shader = &shader;
                        entity.transform.position = vec3(x, y, z);
                        world.Spawn(entity);
                        break;

                    /*
                        This is red wool texture... or was
                        I changed it to white since roof looked better white I am just lazy and dont want to adjust the level.map file
                    */
                    case 6:
                        entity.tag = "red_wool";
                        entity.albedo = &red_wool;
                        entity.specular = &red_wool;
                        /* 
                            Using different specutar materials by using the same texture as the albedo
                        */
                        entity.model = &cubeModel;
                        entity.shader = &shader;
                        entity.transform.position = vec3(x, y, z);
                        world.Spawn(entity);
                        break;
                    /*
                        Glass texture.
                    */
                    case 7:
                        entity.tag = "glass";
                        entity.albedo = &glass;
                        entity.specular = &glass;
                        /* 
                            Using different specutar materials by using the same texture as the albedo
                        */
                        entity.model = &cubeModel;
                        entity.shader = &shader;
                        entity.transform.position = vec3(x, y, z);
                        world.Spawn(entity);
                        break;

                    /*
                        Cherry Leaves texture.
                    */
                    case 8:
                        entity.tag = "cherry_leaves";
                        entity.albedo = &cherry_leaves;
                        entity.specular = &cherry_leaves;
                        /* 
                            Using different specutar materials by using the same texture as the albedo
                        */
                        entity.model = &cubeModel;
                        entity.shader = &shader;
                        entity.transform.position = vec3(x, y, z);
                        world.Spawn(entity);
                        break;

                    /*
                        Netherrack texture.
                    */
                    case 9:
                        entity.tag = "netherrack";
                        entity.albedo = &netherrack;
                        entity.specular = &netherrack;
                        /* 
                            Using different specutar materials by using the same texture as the albedo
                        */
                        entity.model = &cubeModel;
                        entity.shader = &shader;
                        entity.transform.position = vec3(x, y, z);
                        world.Spawn(entity);
                        break;
                        
                    /*
                        Fire texture.
                    */
                    case 10:
                        entity.tag = "fire";
                        entity.albedo = &fireTextures[0];
                        /* 
                            Set the fire to the first texture picture png thing
                        */
                        entity.specular = &textureSpecular;
                        /* 
                            Using different specutar materials by using the same texture as the albedo
                        */
                        entity.model = &grassModel;
                        entity.shader = &shader;
                        entity.transform.position = vec3(x, y, z);
                        world.Spawn(entity);
                        break;

                    case 11:
                        entity.tag = "stone_bricks";
                        entity.albedo = &stone_bricks;
                        entity.specular = &stone_bricks;
                        /* 
                            Using different specutar materials by using the same texture as the albedo
                        */
                        entity.model = &cubeModel;
                        entity.shader = &shader;
                        entity.transform.position = vec3(x, y, z);
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

    /*
        The timer adjusts each time the fire logic is iterated through.
        The fire frame keeps track of the current texture png and gets the next and uses modulus NUM_FIRE_FRAMES to ensure it doesn't go out of bounds.
    */
    const float FIRE_FRAME_DURATION = 0.5f;
    /*
        0.5f Frame duration = my 15 assignment points
    */
    float fireTimer = 0.0f;
    int fireFrame = 0;

    while (inputManager.Update(Canis::GetConfig().width, Canis::GetConfig().heigth))
    {
        deltaTime = frameRateManager.StartFrame();
        Canis::Graphics::ClearBuffer(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);

        fireTimer += deltaTime;
        /* 
            Using the already coded delta time and just adding it
        */

        if (fireTimer >= FIRE_FRAME_DURATION) {
            /*
                If the fireTImer has been added delta time enough and is greater than the number that is compaired then run the logic
            */
            fireFrame = (fireFrame + 1) % NUM_FIRE_FRAMES;
            /*
                Get the next texture. The modulus ensures it doesn't go out of bounds and loops back
            */
            for (auto it = world.m_entities.begin(); it != world.m_entities.end(); ++it) {
                /*
                    Lets just curse this and iterate through every single m_entity and check if it has the fire tag. Had to make m_entity public too
                */
                if (it->tag == "fire") {
                    /*
                        Ding ding ding! We have a match! Now we can update the texture of the fire entity
                    */
                    Canis::Entity updatedEntity = *it;
                    /*
                        Gonna copy that current entity.
                    */
                    updatedEntity.albedo = &fireTextures[fireFrame];
                    /*
                        Add the super new texture to the copied entity
                    */
                    it = world.m_entities.erase(it);
                    /*
                        Nuclear explosion! Erase the old entity and add the new one with the updated texture
                    */
                    world.m_entities.push_back(updatedEntity);
                    /*
                        Put the handsome updated entity back into the m_entities vector
                    */
                }
            }
            fireTimer = 0.0f;
            /*
                It is me Dr. Fire Timer! I am back to 0.0f! I am so happy to be here!
            */
        }
    
        world.Update(deltaTime);
        world.Draw(deltaTime);
    
        editor.Draw();
    
        window.SwapBuffer();
    
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


    

    //pointLight.position = vec3(4.0f, 1.0f, 7.0f);
    //pointLight.ambient = vec3(0.2f, 0.2f, 0.2f);

    //_world.SpawnPointLight(pointLight);
}
