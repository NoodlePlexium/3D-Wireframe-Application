#pragma once

#include <SFML/Graphics.hpp>
#include "libs/glm/glm.hpp"
#include "camera.h"
#include <string>
#include <vector>
#include <iostream>
#include <stdexcept> 
#include <cmath> 
#include <omp.h>
#include "mesh.hpp"

glm::vec2 LineLineIntersection(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4)
{
    glm::vec2 intersection = {-1.0f, -1.0f};

    // CALCULATE THE LINE DIRECTIONS
    float uA = ((x4-x3)*(y1-y3) - (y4-y3)*(x1-x3)) / ((y4-y3)*(x2-x1) - (x4-x3)*(y2-y1));
    float uB = ((x2-x1)*(y1-y3) - (y2-y1)*(x1-x3)) / ((y4-y3)*(x2-x1) - (x4-x3)*(y2-y1));

    // LINE COLLISION CHECK (IF UA & UB ARE BETWEEN 0-1)
    if (uA >= 0 && uA <= 1 && uB >= 0 && uB <= 1) {

        // CALCULATE INTERSECTION COORDINATE
        intersection = {x1 + (uA * (x2-x1)), y1 + (uA * (y2-y1))};
    }
    return intersection;
}

glm::vec4 LineWindowIntersection(float x1, float y1, float x2, float y2, float w, float h)
{
    // TEST EACH WINDOW BORDER FOR AN INTERSECTION
    glm::vec2 leftIntersection   = LineLineIntersection(x1, y1, x2, y2, 0, 0, 0, h);
    glm::vec2 rightIntersection  = LineLineIntersection(x1, y1, x2, y2, w, 0, w, h);
    glm::vec2 topIntersection    = LineLineIntersection(x1, y1, x2, y2, 0, 0, w, 0);
    glm::vec2 bottomIntersection = LineLineIntersection(x1, y1, x2, y2, 0, h, w, h);
    glm::vec4 intersections = {-1.0f, -1.0f, -1.0f, -1.0f};

    if (leftIntersection.x != -1.0f)
    {
        intersections.x = leftIntersection.x;
        intersections.y = leftIntersection.y;
    }
    if (topIntersection.x != -1.0f)
    {
        intersections.x = topIntersection.x;
        intersections.y = topIntersection.y; 
    }
    if (rightIntersection.x != -1.0f)
    {
        intersections.z = rightIntersection.x;
        intersections.w = rightIntersection.y;
    }
    if (bottomIntersection.x != -1.0f)
    {
        intersections.z = bottomIntersection.x;
        intersections.w = bottomIntersection.y;
    }
    return intersections;
}

glm::vec2 LineInWindowIntersection(float x1, float y1, float x2, float y2, float w, float h)
{
    // TEST EACH WINDOW BORDER FOR AN INTERSECTION
    glm::vec2 leftIntersection   = LineLineIntersection(x1, y1, x2, y2, 0, 0, 0, h);
    glm::vec2 rightIntersection  = LineLineIntersection(x1, y1, x2, y2, w, 0, w, h);
    glm::vec2 topIntersection    = LineLineIntersection(x1, y1, x2, y2, 0, 0, w, 0);
    glm::vec2 bottomIntersection = LineLineIntersection(x1, y1, x2, y2, 0, h, w, h);
    glm::vec2 intersection = {-1.0f, -1.0f};

    if (leftIntersection.x != -1.0f)
    {
        intersection.x = leftIntersection.x;
        intersection.y = leftIntersection.y;
    }
    if (topIntersection.x != -1.0f)
    {
        intersection.x = topIntersection.x;
        intersection.y = topIntersection.y; 
    }
    if (rightIntersection.x != -1.0f)
    {
        intersection.x = rightIntersection.x;
        intersection.y = rightIntersection.y;
    }
    if (bottomIntersection.x != -1.0f)
    {
        intersection.x = bottomIntersection.x;
        intersection.y = bottomIntersection.y;
    }
    return intersection;
}

void DrawLine2D(const glm::vec2 &v1, const glm::vec2 &v2, sf::Image &image)
{
    int imageWidth = image.getSize().x;
    int imageHeight = image.getSize().y;
    
    // INITIALISE TRACKING COORDINATES
    float x0 = v1.x;
    float y0 = v1.y;

    // HORIZONTAL AND VERTICAL DISTANCE COMPONENTS
    float dx = std::abs(v2.x - x0);
    float dy = std::abs(v2.y - y0);

    // DETERMINE STEP DIRECTION
    int sx = (x0 < v2.x) ? 1 : -1;
    int sy = (y0 < v2.y) ? 1 : -1;
    float err = dx - dy;
    float distance = std::sqrt(dx * dx + dy * dy); 

    while (true)
    {
        // CHECK IF PIXEL COORDINATES ARE WINTHIN IMAGE BOUNDS
        int px = static_cast<int>(x0);
        int py = static_cast<int>(y0);
        if (px >= 0 && px < imageWidth && py >= 0 && py < imageHeight)
        {
            image.setPixel(px, py, sf::Color::White);
        }

        // DETERMINE NEXT STEP
        float e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }

        // END CONDITION (DISTANCE TRAVELLED >= LINE DISTANCE)
        float distTraveled = std::sqrt((x0 - v1.x) * (x0 - v1.x) + (y0 - v1.y) * (y0 - v1.y)); 
        if (distTraveled >= distance)
            break;
    }
}

void DrawWireframe(const Mesh &mesh, Camera &camera, sf::Image &image)
{
    int imageWidth = image.getSize().x;
    int imageHeight = image.getSize().y;

    // CALCULATE MODEL VIEW PROJECTION
    glm::mat4 modelMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    glm::mat4 mvp = camera.ProjectionViewMatrix() * modelMat;

    // DRAW EACH FACE IN PARALLEL (3 EDGES)
    #pragma omp parallel for
    for (int i = 0; i < mesh.indices.size(); i += 3)
    {
        // VERTICES FROM MESH VERTEX FLOATS
        glm::vec3 v1 = glm::vec3(mesh.vertices[mesh.indices[i] * 3], mesh.vertices[mesh.indices[i] * 3 + 1], mesh.vertices[mesh.indices[i] * 3 + 2]);
        glm::vec3 v2 = glm::vec3(mesh.vertices[mesh.indices[i + 1] * 3], mesh.vertices[mesh.indices[i + 1] * 3 + 1], mesh.vertices[mesh.indices[i + 1] * 3 + 2]);
        glm::vec3 v3 = glm::vec3(mesh.vertices[mesh.indices[i + 2] * 3], mesh.vertices[mesh.indices[i + 2] * 3 + 1], mesh.vertices[mesh.indices[i + 2] * 3 + 2]);

        // APPLY MODEL VIEW PROJECTION TRANSFORMATION (CONVERT TO CAMERA SPACE)
        glm::vec4 v1_transformed = mvp * glm::vec4(v1, 1.0f);
        glm::vec4 v2_transformed = mvp * glm::vec4(v2, 1.0f);
        glm::vec4 v3_transformed = mvp * glm::vec4(v3, 1.0f);

        // PERFORM PERSPECTIVE DIVISION (HANDLE W = 0 CASE LATER)
        glm::vec3 v1_ndc = glm::vec3(v1_transformed) / v1_transformed.w;
        glm::vec3 v2_ndc = glm::vec3(v2_transformed) / v2_transformed.w;
        glm::vec3 v3_ndc = glm::vec3(v3_transformed) / v3_transformed.w;

        // BACKFACE CULLING CHECK
        glm::vec3 faceNormal = glm::cross(v2 - v1, v3 - v1); // Compute face normal
        glm::vec3 camToVertex = v1 - camera.position; // Compute vector from camera to one of the face vertices
        if (glm::dot(faceNormal, camToVertex) < 0.0f)
        {
            // CHECK WHICH VERTICES ARE WITHIN THE NDC BOUNDS
            int inCount = 0;
            bool v1In = false;
            bool v2In = false;
            bool v3In = false;
            if (glm::all(glm::greaterThanEqual(v1_ndc, glm::vec3(-1.0f))) && glm::all(glm::lessThanEqual(v1_ndc, glm::vec3(1.0f)))) {
                inCount += 1;
                v1In = true;
            }
            if (glm::all(glm::greaterThanEqual(v2_ndc, glm::vec3(-1.0f))) && glm::all(glm::lessThanEqual(v2_ndc, glm::vec3(1.0f)))) {
                inCount += 1;
                v2In = true;
            }
            if (glm::all(glm::greaterThanEqual(v3_ndc, glm::vec3(-1.0f))) && glm::all(glm::lessThanEqual(v3_ndc, glm::vec3(1.0f)))) {
                inCount += 1;
                v3In = true;
            }

            if (inCount == 3)
            {
                // CONVERT NDC TO SCREEN SPACE
                glm::vec2 v1_screen = glm::vec2((v1_ndc.x + 1.0f) * 0.5f * imageWidth, (1.0f - v1_ndc.y) * 0.5f * imageHeight);
                glm::vec2 v2_screen = glm::vec2((v2_ndc.x + 1.0f) * 0.5f * imageWidth, (1.0f - v2_ndc.y) * 0.5f * imageHeight);
                glm::vec2 v3_screen = glm::vec2((v3_ndc.x + 1.0f) * 0.5f * imageWidth, (1.0f - v3_ndc.y) * 0.5f * imageHeight);

                // DRAW EDGES
                DrawLine2D(v1_screen, v2_screen, image); // draw edge v1 v2
                DrawLine2D(v1_screen, v3_screen, image); // draw edge v1 v3
                DrawLine2D(v2_screen, v3_screen, image); // draw edge v2 v3
            }

            // FORM A QUAD
            if (inCount == 2)
            {
                glm::vec2 v1_screen = glm::vec2((v1_ndc.x + 1.0f) * 0.5f * imageWidth, (1.0f - v1_ndc.y) * 0.5f * imageHeight);
                glm::vec2 v2_screen = glm::vec2((v2_ndc.x + 1.0f) * 0.5f * imageWidth, (1.0f - v2_ndc.y) * 0.5f * imageHeight);
                glm::vec2 v3_screen = glm::vec2((v3_ndc.x + 1.0f) * 0.5f * imageWidth, (1.0f - v3_ndc.y) * 0.5f * imageHeight);

                // V1 IS OUTSIDE THE NDC
                if (!v1In)
                {
                    glm::vec2 v1v2_screen = LineInWindowIntersection(v1_screen.x, v1_screen.y, v2_screen.x, v2_screen.y, imageWidth, imageHeight);
                    glm::vec2 v1v3_screen = LineInWindowIntersection(v1_screen.x, v1_screen.y, v3_screen.x, v3_screen.y, imageWidth, imageHeight);
                    DrawLine2D(v1v2_screen, v2_screen, image); // draw edge "v1v2 intersection" v2
                    DrawLine2D(v1v3_screen, v3_screen, image); // draw edge "v1v3 intersection" v3
                    DrawLine2D(v2_screen, v3_screen, image); // draw edge v2 v3
                }

                // V2 IS OUTSIDE THE NDC
                if (!v2In)
                {
                    glm::vec2 v2v1_screen = LineInWindowIntersection(v2_screen.x, v2_screen.y, v1_screen.x, v1_screen.y, imageWidth, imageHeight);
                    glm::vec2 v2v3_screen = LineInWindowIntersection(v2_screen.x, v2_screen.y, v3_screen.x, v3_screen.y, imageWidth, imageHeight);
                    DrawLine2D(v2v1_screen, v1_screen, image); // draw edge "v2v1 intersection" v1
                    DrawLine2D(v2v3_screen, v3_screen, image); // draw edge "v2v3 intersection" v3
                    DrawLine2D(v1_screen, v3_screen, image); // draw edge v1 v3
                }

                // V3 IS OUTSIDE THE NDC
                if (!v3In)
                {
                    glm::vec2 v3v2_screen = LineInWindowIntersection(v3_screen.x, v3_screen.y, v2_screen.x, v2_screen.y, imageWidth, imageHeight);
                    glm::vec2 v3v1_screen = LineInWindowIntersection(v3_screen.x, v3_screen.y, v1_screen.x, v1_screen.y, imageWidth, imageHeight);
                    DrawLine2D(v3v2_screen, v2_screen, image); // draw edge "v3v2 intersection" v2
                    DrawLine2D(v3v1_screen, v1_screen, image); // draw edge "v3v1 intersection" v1
                    DrawLine2D(v2_screen, v1_screen, image); // draw edge v2 v1
                }
            }

            // FORM TRIANGLE
            if (inCount == 1)
            {
                glm::vec2 v1_screen = glm::vec2((v1_ndc.x + 1.0f) * 0.5f * imageWidth, (1.0f - v1_ndc.y) * 0.5f * imageHeight);
                glm::vec2 v2_screen = glm::vec2((v2_ndc.x + 1.0f) * 0.5f * imageWidth, (1.0f - v2_ndc.y) * 0.5f * imageHeight);
                glm::vec2 v3_screen = glm::vec2((v3_ndc.x + 1.0f) * 0.5f * imageWidth, (1.0f - v3_ndc.y) * 0.5f * imageHeight);

                // V1 IS INSIDE THE NDC
                if (v1In)
                {
                    glm::vec2 v1v2_screen = LineInWindowIntersection(v2_screen.x, v2_screen.y, v1_screen.x, v1_screen.y, imageWidth, imageHeight);
                    glm::vec2 v1v3_screen = LineInWindowIntersection(v3_screen.x, v3_screen.y, v1_screen.x, v1_screen.y, imageWidth, imageHeight);
                    DrawLine2D(v1v2_screen, v1_screen, image); // draw edge "v1v2 intersection" v2
                    DrawLine2D(v1v3_screen, v1_screen, image); // draw edge "v1v3 intersection" v3
                }

                // V2 IS INSIDE THE NDC
                if (v2In)
                {
                    glm::vec2 v2v1_screen = LineInWindowIntersection(v1_screen.x, v1_screen.y, v2_screen.x, v2_screen.y, imageWidth, imageHeight);
                    glm::vec2 v2v3_screen = LineInWindowIntersection(v3_screen.x, v3_screen.y, v2_screen.x, v2_screen.y, imageWidth, imageHeight);
                    DrawLine2D(v2v1_screen, v2_screen, image); // draw edge "v2v1 intersection" v1
                    DrawLine2D(v2v3_screen, v2_screen, image); // draw edge "v2v1 intersection" v3
                }

                // V3 IS INSIDE THE NDC
                if (v3In)
                {
                    glm::vec2 v3v2_screen = LineInWindowIntersection(v2_screen.x, v2_screen.y, v3_screen.x, v3_screen.y, imageWidth, imageHeight);
                    glm::vec2 v3v1_screen = LineInWindowIntersection(v1_screen.x, v1_screen.y, v3_screen.x, v3_screen.y, imageWidth, imageHeight);
                    DrawLine2D(v3v2_screen, v3_screen, image); // draw edge "v1v2 intersection" v2
                    DrawLine2D(v3v1_screen, v3_screen, image); // draw edge "v3v1 intersection" v1
                }
            }
        }
    }
}


