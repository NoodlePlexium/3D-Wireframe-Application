# Compile main.cpp and create main.o
& g++ -c main.cpp -I"libs\SFML\include"

# Link main.o and create the application executable
& g++ -o build\application main.cpp -I"libs\SFML\include" -L"libs\SFML\lib" -fopenmp -lsfml-graphics -lsfml-window -lsfml-system 
# Delete the main.o file
Remove-Item "main.o" -Force
