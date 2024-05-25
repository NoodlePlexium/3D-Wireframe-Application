#pragma once

#include <vector> 
#include <string>
#include <fstream>
#include <iostream>
#include "mesh.hpp"

// GENERATE NEW LIST OF TRIANGULATED INDICES FROM A LIST OF POLYGON INDICES
std::vector<unsigned int> TriangulatePolygon(std::vector<unsigned int> &indices)
{
    std::vector<unsigned int> triangulatedIndices;
    for (int i=0; i<indices.size()-2; ++i)
    {
        triangulatedIndices.push_back(indices[0]);
        triangulatedIndices.push_back(indices[i+1]);
        triangulatedIndices.push_back(indices[i+2]);
    }
    return triangulatedIndices;
}

// LOAD OBJ FROM FILE INTO MESH CLASS
Mesh LoadOBJ(std::string filepath)
{
    Mesh mesh;
    std::ifstream file(filepath);



    // CHECK IF FILEPATH EXISTS
    if (!file.is_open())
    {
        std::cerr << "[LoadOBJ] Error: Could not open file '" << filepath << "'" << std::endl;
    }



    // ITERATE OVER LINES
    std::string line;
    while (std::getline(file, line))
    {   


        
        // TOKENISE THE LINE BY " " (SPACE DELIMITER)
        std::vector<std::string> tokens;
        std::string token;
        for (char ch : line)
        {
            if (ch == ' ')
            {
                tokens.push_back(token);
                token = "";
            }
            else
            {
                token += ch;
            }
        }
        tokens.push_back(token); // append the last token



        // LINE CORRESPONDS TO A VERTEX
        if (tokens[0] == "v")
        {
            for (int i=1; i<tokens.size(); ++i)
            {
                mesh.vertices.push_back(std::stof(tokens[i])); 
            }
        }



        // LINE CORRESPONDS TO A FACE
        if (tokens[0] == "f")
        {
            // EXTRACT INDICES FROM LINE
            std::vector<unsigned int> newIndices;
            for (int i=1; i<tokens.size(); ++i)
            {
                std::string indexString;

                // EXTRACT FIRST NUMBER (VERTEX INDEX)
                for (char ch : tokens[i])
                {
                    if (ch == '/') break;
                    else {
                        indexString += ch;
                    }
                }

                // APPEND TO LIST OF NEW INDICES
                unsigned int index = std::stoul(indexString) - 1;
                newIndices.push_back(index);
            }

            // TRIANGULAR FACE
            if (newIndices.size() == 3)
            {
                for (int i=0; i<newIndices.size(); ++i)
                {
                    mesh.indices.push_back(newIndices[i]);
                }
            }
            // POLIGONAL FACE MUST BE TRIANGULATED
            else
            {
                std::vector<unsigned int> triangulatedIndices = TriangulatePolygon(newIndices);

                // APPEND TRIANGULATED INDICES TO MODEL
                for (int i=0; i<triangulatedIndices.size(); ++i)
                {
                    mesh.indices.push_back(triangulatedIndices[i]);
                }
            }
        }
    }
    


    // CLOSE FILE AND RETURN MESH
    file.close();
    return mesh;
}
