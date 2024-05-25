#include <SFML/Graphics.hpp>
#include <iostream>
#include <chrono>
#include <omp.h>
#include "Input.h"
#include "mesh.hpp"
#include "loader.hpp"
#include "RenderSystem.hpp"
#include "../libs/glm/glm.hpp"

struct GLOBAL
{
    int WIDTH = 800;
    int HEIGHT = 600;
    float FRAME_TIME;
};

// GLOBAL VARIABLES
GLOBAL global;
sf::RenderWindow window(sf::VideoMode(global.WIDTH, global.HEIGHT), "Wireframe Engine");    
InputSystem Input{&window};
Camera camera;        
sf::Image renderImage; 



void ProcessInput() 
{
    // CLEAR INPUT SYSTEM STATE
    Input.NewFrame();

    sf::Event event;
    while (window.pollEvent(event)) 
    {
        // CLOSE WINDOW EVENT
        if (event.type == sf::Event::Closed) window.close();

        // RESIZE WINDOW EVENT
        else if (event.type == sf::Event::Resized) 
        {
            // ADJUST THE VIEW ON WINDOW RESIZE
            sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
            window.setView(sf::View(visibleArea));

            // APPLY MINIMUM WINDOW SIZE
            window.setSize(sf::Vector2u(
                static_cast<unsigned int>(std::max(static_cast<int>(window.getSize().x), 100)),
                static_cast<unsigned int>(std::max(static_cast<int>(window.getSize().y), 400))
            ));       

            // UPDATE GLOBAL WINDO SIZE VARIABLES
            global.WIDTH = static_cast<int>(window.getSize().x);
            global.HEIGHT = static_cast<int>(window.getSize().y);

            // UPDATE CAMERA MATRIX
            camera.SetViewport(global.WIDTH, global.HEIGHT);
            camera.UpdateProjectionView();
        }

        // INPUT SYSTEM HANDLE IO EVENTS
        Input.HandleEvent(event);
    } 
    // UPDATE INPUT SYSTEM STATE
    Input.UpdateKeyStates();
}

void Init()
{
    // INITIALISE MOUSE CURSOR 
    window.setMouseCursorVisible(true);
    Input.ShowMouse();

    // INITIALISE RENDER IMAGE
    renderImage.create(global.WIDTH, global.HEIGHT, sf::Color::Black); 

    // INITIALISE CAMERA
    camera.SetViewport(global.WIDTH, global.HEIGHT);
    camera.position = {2.0f, 28.0f, 8.0f};
    camera.UpdateProjectionView(); 
}

int main() {

    // INITIALIZE
    Init();

    // LOAD OBJ AS MESH
    Mesh mesh = LoadOBJ("./models/minecraft.obj");




    // MAIN UPDATE LOOP
    while (window.isOpen()) 
    {
        auto start = std::chrono::high_resolution_clock::now();
        ProcessInput();

        // CLEAR RENDER TARGET
        renderImage.create(global.WIDTH, global.HEIGHT, sf::Color::Black);

        // TOGGLE MOUSE
        if (Input.GetKeyDown(KeyCode::Escape))
        {
            if (Input.MouseHidden())
            {
                // SHOW MOUSE
                window.setMouseCursorVisible(true);
                Input.ShowMouse();
            }
            else
            {   
                // HIDE MOUSE
                window.setMouseCursorVisible(false);
                Input.LockMouse(window.getSize().x / 2, window.getSize().y / 2);
            }
        }

        // BASIC CAMERA MOVEMENT
        if (Input.GetKey(KeyCode::W)) camera.position += 6.5f * camera.Forward() * global.FRAME_TIME;
        if (Input.GetKey(KeyCode::A)) camera.position -= 6.5f * camera.Right() * global.FRAME_TIME;
        if (Input.GetKey(KeyCode::S)) camera.position -= 6.5f * camera.Forward() * global.FRAME_TIME;
        if (Input.GetKey(KeyCode::D)) camera.position += 6.5f * camera.Right() * global.FRAME_TIME;
        if (Input.GetKey(KeyCode::E)) camera.position += 6.5f * glm::vec3(0.0f, 1.0f, 0.0f) * global.FRAME_TIME;
        if (Input.GetKey(KeyCode::Q)) camera.position -= 6.5f * glm::vec3(0.0f, 1.0f, 0.0f) * global.FRAME_TIME;

        // BASIC CAMERA LOOK
        if (Input.MouseHidden())
        {
            float dX = Input.MouseDeltaX() * 0.3f;
            float dY = Input.MouseDeltaY() * 0.3f;

            camera.rotation.y += dX;
            camera.rotation.x += dY;
            camera.rotation.x = std::max(-89.0f, std::min(89.0f, camera.rotation.x));
        }
        camera.UpdateProjectionView(); 

        // RENDER MESH AS WIREFRAME (RENDER PIPELINE)
        DrawWireframe(mesh, camera, renderImage);

        // CREATE SPRITE FROM RENDER IMAGE
        sf::Texture texture;
        sf::Sprite sprite;
        texture.loadFromImage(renderImage);
        sprite.setTexture(texture);
        
        // WINDOW DRAW SPRITE
        window.clear();
        window.draw(sprite);
        window.display();

        // FRAME TIME CALCULATION
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        global.FRAME_TIME = duration.count();
    }



    return EXIT_SUCCESS;
}
