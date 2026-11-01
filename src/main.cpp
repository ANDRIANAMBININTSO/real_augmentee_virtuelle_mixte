#include <GLFW/glfw3.h>
#include <iostream>
#include "detection.hpp"
#include "video.hpp"

using namespace std;
using namespace cv;
 

 

int main() {
 
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        return 1;
    }
    
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Hello", NULL, NULL);
    if (!window) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    }
    // window devient la fenêtre sur laquelle on travaille et fait les modifications
    glfwMakeContextCurrent(window);
    
    
    // pour ouvrir la caméra 
    //Video video(0);
    // pour ouvir une vidéo 
    Video video("../input/input_1.mp4");
    
    // checker les fps de la video : 30 fps
    double fps = video.fps();
    cout << "Frames per seconds : " << fps << endl;
    
    // identifiant de la texture
    GLuint textureID;
    // génère un identifiant de texture et le range dans textureID
    glGenTextures(1, &textureID);
    // la texture devient active
    glBindTexture(GL_TEXTURE_2D, textureID);
    //gère l'échantillonage (plus ou moins de pixel texture que de pixel fenêtre)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // dis a opengl d'utiliser l'image définie dans glGenTextures et glBindTexture lors du rendu
    glEnable(GL_TEXTURE_2D);
    // pour les videos dont la largeur n'est pas multiple de 4
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    double lastFrameTime = 0;
    double currentTime;
    double dureeFrame = 1.0/fps;
    Mat frame;
    Mat binary;
    // tant qu'on ne ferme pas la fenêtre 
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
    
        // on récupère le temps écoulé depuis le début
        currentTime = glfwGetTime();
        // on doit changer d'image
        if (currentTime - lastFrameTime >= dureeFrame) {
            // on stocke l'image dans une matrice
            if (!(video.read(frame))) {
                break;
            }
            //lastFrameTime = currentTime; // on ne fait pas ça car ça ne nous permet pas de rattraper le retard s'il y en a
            lastFrameTime += dureeFrame;
        
            //Traitement complet d'une frame
            binary = detection::segmentSheet(frame);
        
            //Si on arrive à trouver le contour le plus grand(la feuille)
            vector<Point> largest;
            if (detection::findLargestContour(binary, largest)) {
                //On cherche les coins
                vector<Point2f> corners = detection::findCorners(largest);
                //On les dessine
                detection::drawSheet(frame, corners);
            }
        
            // OpenGL a l'origine en bas, OpenCV en haut : on retourne l'image juste avant l'envoi
            cv::flip(frame, frame, 0);
        
            // remplis la texture avec ces pixels
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame.cols, frame.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, frame.data);
        }
    
        // on dessine le carré dans lequel on va mettre la texture
        // pour pouvoir afficher toutes les videos (peut importe le format) correctement
        // ratio de la fenêtre et de la vidéo
        int winW, winH;
        glfwGetFramebufferSize(window, &winW, &winH);
        double winRatio = (double)winW / winH;
        double vidRatio = (double)frame.cols / frame.rows;

        // par défaut, on remplit tout
        float sx = 1.0f, sy = 1.0f;

        if (vidRatio > winRatio) {
            // vidéo plus large que la fenêtre : bandes en haut et en bas
            sy = winRatio / vidRatio;
        } else {
            // vidéo plus haute que la fenêtre : bandes sur les côtés
            sx = vidRatio / winRatio;
        }

        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(-sx, -sy);
        glTexCoord2f(1, 0); glVertex2f( sx, -sy);
        glTexCoord2f(1, 1); glVertex2f( sx,  sy);
        glTexCoord2f(0, 1); glVertex2f(-sx,  sy);
        glEnd();
    
        // back buffer (dessin) devient le front buffer (visible)
        glfwSwapBuffers(window);
        // récupère les évènement (clics, souris, etc)
        glfwPollEvents();
    }
    cout << glfwGetTime() << endl;
    // nettoyage
    glDeleteTextures(1, &textureID);
        
    // ferme proprement toutes les ressources de GLFW (pendant de glfwInit())
    glfwTerminate();
    return 0;
}
 
