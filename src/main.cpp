#include <GLFW/glfw3.h>
#include <iostream>
#include <opencv2/calib3d.hpp>

#include "detection.hpp"
#include "video.hpp"

using namespace std;
using namespace cv;
 

GLuint createTexture() {
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

    return textureID;
}

void drawImage(GLFWwindow* w, GLuint tex, const Mat& f) {
    glfwMakeContextCurrent(w);

    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, tex);
    // OpenGL a l'origine en bas, OpenCV en haut : on retourne l'image juste avant l'envoi
    Mat flipped;
    flip(f, flipped, 0);
        
    // remplis la texture avec ces pixels
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, flipped.cols, flipped.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, flipped.data);

    // on dessine le carré dans lequel on va mettre la texture
    // pour pouvoir afficher toutes les videos (peut importe le format) correctement
    // ratio de la fenêtre et de la vidéo
    int winW, winH;
    glfwGetFramebufferSize(w, &winW, &winH);
    double winRatio = (double)winW / winH;
    double vidRatio = (double)f.cols / f.rows;

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
    glfwSwapBuffers(w);
}



int main() {
 
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        return 1;
    }
    
    GLFWwindow* window = glfwCreateWindow(1280, 720, "main", NULL, NULL);
    if (!window) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    }
    GLFWwindow* virtualWindow = glfwCreateWindow(1280, 720, "virtualPlan", NULL, NULL);
    if (!virtualWindow) {
        fprintf(stderr, "ERROR: could not open virtualWindow with GLFW3\n");
        glfwTerminate();
        return 1;
    }
    // window devient la fenêtre sur laquelle on travaille et fait les modifications
    glfwMakeContextCurrent(window);
    GLuint mainTexture = createTexture();

    glfwMakeContextCurrent(virtualWindow);
    GLuint virtualTexture = createTexture();

    glfwMakeContextCurrent(window);
    
    
    // pour ouvrir la caméra 
    //Video video(0);
    // pour ouvir une vidéo 
    Video video("../input/input.mp4");
    
    // checker les fps de la video : 30 fps
    double fps = video.fps();
    cout << "Frames per seconds : " << fps << endl;
    
    double lastFrameTime = 0;
    double currentTime;
    double dureeFrame = 1.0/fps;
    Mat frame, virtualFrame;
    Mat binary;
    vector<Point2f> prevCorners;


    // tant qu'on ne ferme pas la fenêtre 
    while (!glfwWindowShouldClose(window)) {
        //glClear(GL_COLOR_BUFFER_BIT);
    
        // on récupère le temps écoulé depuis le début
        currentTime = glfwGetTime();
        // on doit changer d'image
        if (currentTime - lastFrameTime >= dureeFrame) {
            // on stocke l'image dans une matrice
            if (!(video.read(frame, true))) {
                break;
            }
            //lastFrameTime = currentTime; // on ne fait pas ça car ça ne nous permet pas de rattraper le retard s'il y en a
            lastFrameTime += dureeFrame;

            virtualFrame = Mat::zeros(frame.size(), CV_8UC3);
        
            //Traitement complet d'une frame
            binary = detection_amelioree::segmentSheet(frame);
            // affichage de la video en binaire
            //cv::imshow("binary", binary);

        
            //Si on arrive à trouver le contour le plus grand(la feuille)
            vector<Point> largest;
            if (detection::findLargestContour(binary, largest)) {
                //On cherche les coins
                vector<Point2f> corners = detection::findCorners(largest, prevCorners);
                //On les dessine
                detection::drawSheet(frame, corners);


                // création des points virtuels, coordonnées locales
                vector<Point3f> virtualPoints = {
                    {-148.5f, 105.f, 0.f}, // point haut gauche
                    {148.5f, 105.f, 0.f}, // point haut droite
                    {148.5f, -105.f, 0.f}, // point bas droite
                    {-148.5f, -105.f, 0.f} // point bas gauche
                };
                Point3f sheetCenter(0.f, 0.f, 0.f);
                
                double f = frame.cols;
                // Matrice de la caméra
                Mat cameraMatrix = (Mat_<double>(3,3) <<
                    f, 0, frame.cols / 2.0,
                    0, f, frame.rows / 2.0,
                    0, 0, 1);

                // Matrice de distorsion (pas de distorsion)
                Mat distCoeffs = Mat::zeros(4, 1, CV_64F);
                // Matrices de rotation et de translation
                Mat rvec, tvec;
                // transforme le 2D en 3D pour la caméra
                bool is2D = solvePnP(virtualPoints, corners, cameraMatrix, distCoeffs, rvec, tvec);

                if (is2D) {
                    // dessin du repère
                    drawFrameAxes(frame, cameraMatrix, distCoeffs, rvec, tvec, 100.f);

                    // application de la formule P_cam = R · P_obj + t avec correctionde distorsion et conversion en pixels
                    vector<Point2f> pixels;
                    projectPoints(virtualPoints, rvec, tvec, cameraMatrix, distCoeffs, pixels);
                    // on récupère une liste de coordonnées 2D, une par points 3D
                    // il nous reste plus qu'a les dessiner
                    vector<Point> poly(pixels.begin(), pixels.end());
                    fillConvexPoly(virtualFrame, poly, Scalar(0,0,255));
                }
            }
        }
       
        drawImage(window, mainTexture, frame);
        drawImage(virtualWindow, virtualTexture, virtualFrame);
        // récupère les évènement (clics, souris, etc)
        glfwPollEvents();
    }
    cout << glfwGetTime() << endl;
    // nettoyage
    glfwMakeContextCurrent(window);
    glDeleteTextures(1, &mainTexture);
    glfwMakeContextCurrent(virtualWindow);
    glDeleteTextures(1, &virtualTexture);
        
    // ferme proprement toutes les ressources de GLFW (pendant de glfwInit())
    glfwTerminate();
    return 0;
}
 
