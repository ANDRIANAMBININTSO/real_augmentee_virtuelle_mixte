# real_augmentee_virtuelle_mixte
Ce repo est un projet de 5A dans les matières de Réalité Augmentée/Virtuelle/Mixte
ANDRIANAMBININTSOA Menjatiana Danih
BONNEMAINS Émilie.

# Etape 1 : Trouver les contours de la feuille blanche

- Chaque image de la vidéo est convertie en niveau de gris.
- Ensuite, un **flou Gaussien** y est appliqué.
- Une image binaire (threshold **OTSU**) est ensuite produite.
- L'aire de chaque surface est calculée, on ne garde que celle qui en a la plus grande(c'est la feuille A4).
- Les contours sont calculés à l'aide de **findContours**, et sont renvoyés sous forme de tableaux de points.

# Etape 2 : Trouver et trier les coins de la feuille blanche

Une fois qu'on a les points de contours(sous forme de tableau de points), ils sont approximés avec un polygone(ici un quadrilatère).
- La fonction **approxPolyDP** nous permet d'obtenir les 4 coins du quadrilatère.
- Les 4 coins sont ensuite triés par la fonction **sortcorners**.

![tri de coins](images/tri_coins.png)
# Etape 3 : Placer le repère 