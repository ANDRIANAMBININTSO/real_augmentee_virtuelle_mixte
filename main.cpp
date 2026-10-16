
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d.hpp>
 
#include <GLFW/glfw3.h>
#include <stdio.h>
 
#include <iostream>
using namespace cv;
using namespace std;
 
 
// Trie les 4 coins dans un ordre fixe : haut-gauche, haut-droit, bas-droit, bas-gauche
// (indispensable pour associer chaque coin de l'image au bon coin de la vraie feuille)
static vector<Point2f> sortCorners(const vector<Point2f>& pts) {
  vector<Point2f> sorted(4);
  double minSum = 1e18, maxSum = -1e18, minDiff = 1e18, maxDiff = -1e18;
  for (const Point2f& p : pts) {
    double sum = p.x + p.y;    // petit en haut à gauche, grand en bas à droite
    double diff = p.y - p.x;   // petit en haut à droite, grand en bas à gauche
    if (sum < minSum)   { minSum = sum;   sorted[0] = p; }  // haut-gauche
    if (diff < minDiff) { minDiff = diff; sorted[1] = p; }  // haut-droit
    if (sum > maxSum)   { maxSum = sum;   sorted[2] = p; }  // bas-droit
    if (diff > maxDiff) { maxDiff = diff; sorted[3] = p; }  // bas-gauche
  }
  return sorted;
}
 
// Trouve les 4 coins d'un contour, déjà triés
static vector<Point2f> findCorners(const vector<Point>& contour) {
  // on simplifie le contour en polygone : une feuille doit donner 4 sommets
  vector<Point> approx;
  approxPolyDP(contour, approx, 0.02 * arcLength(contour, true), true);
 
  vector<Point2f> pts;
  if (approx.size() == 4 && isContourConvex(approx)) {
    for (const Point& p : approx) pts.push_back(Point2f(p));
  } else {
    // contour bruité : on se rabat sur le rectangle orienté qui l'englobe (toujours 4 coins)
    Point2f box[4];
    minAreaRect(contour).points(box);
    pts.assign(box, box + 4);
  }
  return sortCorners(pts);
}
 
 
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
 
 
  // on ouvre la video
  VideoCapture video;
  video.open("../input/input.mp4");
  if (!video.isOpened()) {
    fprintf(stderr, "ERROR: could not open video\n");
    return 1;
  }
 
  // checker les fps de la video : 30 fps
  double fps = video.get(CAP_PROP_FPS);
  std::cout << "Frames per seconds : " << fps << endl;
 
  std::cout << video.get(CAP_PROP_FRAME_WIDTH) << " de largeur" << endl;
  std::cout << video.get(CAP_PROP_FRAME_HEIGHT) << " de hauteur" << endl;
 
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
 
  // tant qu'on ne ferme pas la fenêtre (1 boucle, 1 image)
  while (!glfwWindowShouldClose(window)) {
    // fond de couleur
    //glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
    // on efface l'image qui était affiché pour en créer une nouvelle
    glClear(GL_COLOR_BUFFER_BIT);
 
    // on récupère le temps écoulé depuis le début
    currentTime = glfwGetTime();
    double dureeFrame = 1.0/fps;
 
    // on doit changer d'image
    if (currentTime - lastFrameTime >= dureeFrame) {
      // on stocke l'image dans une matrice
      Mat frame;
      Mat grey;
      Mat blur;
      Mat binary;
      video >> frame;
      if (frame.empty()) {
        break;
      }
      //lastFrameTime = currentTime; // on ne fait pas ça car ça ne nous permet pas de rattraper le retard s'il y en a
      lastFrameTime += dureeFrame;
 
 
 
      // 1. PRETRAITEMENT
      // (le flip est fait plus bas, juste avant l'envoi à OpenGL : tout le traitement
      //  se fait donc sur l'image à l'endroit, avec haut-gauche = (0,0))
      // transforme la frame en niveau de gris pour que ce soit moins lourd
      cvtColor(frame, grey, COLOR_BGR2GRAY);
      //flou gaussien
      GaussianBlur(grey, blur, cv::Size(9, 9), 0);
 
      // 2. SEGMENTATION
      // seuil avec otsu : renvoie une image binaire avec seuil choisi automatiquement
      threshold(blur, binary, 0, 255, THRESH_BINARY | THRESH_OTSU);
 
 
      // 3. GEOMETRIE
      vector<vector<Point>> contours;
      double maxArea= 0.0;
      int maxAreaIndex = -1;
      // trouve tous les contours des zones blanches détectées par otsu
      findContours(binary, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
      // on garde la zone qui a la plus grande aire (feuille A4)
      for (int i =0; i<(int)contours.size(); i++) {
        double area = contourArea(contours[i]);
        if (area > maxArea) {
          maxArea = area;
          maxAreaIndex = i;
        }
      }
 
      // 4. COINS + REPERE (seulement si on a trouvé une feuille)
      if (maxAreaIndex >= 0) {
        // les 4 coins triés : 0 = haut-gauche, 1 = haut-droit, 2 = bas-droit, 3 = bas-gauche
        vector<Point2f> corners = findCorners(contours[maxAreaIndex]);
 
        // contour de la feuille (rouge) + coins numérotés pour vérifier le tri
        vector<Point> quad(corners.begin(), corners.end());
        polylines(frame, quad, true, Scalar(0, 0, 255), 2);
        for (int i = 0; i < 4; i++) {
          circle(frame, corners[i], 6, Scalar(0, 255, 255), FILLED);
          putText(frame, to_string(i), corners[i] + Point2f(8, -8),
                  FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 255), 2);
        }
      }
 
      // OpenGL a l'origine en bas, OpenCV en haut : on retourne l'image juste avant l'envoi
      cv::flip(frame, frame, 0);
 
      // remplis la texture avec ces pixels
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame.cols, frame.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, frame.data);
    }
 
    // on dessine le carré dans lequel on va mettre la texture
    glBegin(GL_QUADS);
      glTexCoord2f(0, 0); glVertex2f(-1, -1);   // coin bas gauche
      glTexCoord2f(1, 0); glVertex2f( 1, -1);   // coin bas droit
      glTexCoord2f(1, 1); glVertex2f( 1, 1);   // coin haut droit
      glTexCoord2f(0, 1); glVertex2f(-1, 1);   // coin haut gauche
    glEnd();
 
    // back buffer (dessin) devient le front buffer (visible)
    glfwSwapBuffers(window);
    // récupère les évènement (clics, souris, etc)
    glfwPollEvents();
}
  std::cout << glfwGetTime() << endl;
  // nettoyage
  glDeleteTextures(1, &textureID);
  video.release();
 
  // ferme proprement toutes les ressources de GLFW (pendant de glfwInit())
  glfwTerminate();
  return 0;
}
 
