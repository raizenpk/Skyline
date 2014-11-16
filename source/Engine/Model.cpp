#include "Model.h"

using namespace Engine;

Model::Model() {
}

Model::~Model() {

}

int Model::getHeight() {
  return this->height;
}

int Model::getWidth() {
  return this->width;
}

GLfloat* Model::getUV() {
  return this->uv;
}

GLfloat* Model::getVertices() {
  return this->vertices;
}
