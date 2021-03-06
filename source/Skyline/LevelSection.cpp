#include "LevelSection.h"
#include "Game.h"
#include <algorithm>
#include "EntityManager.h"

LevelSection::LevelSection(float width, float height) {
  _game = Game::GetInstance();
  _width = _game->getGameplayScreen()->getMainCamera()->getWorldScalar(width);
  _height = _game->getGameplayScreen()->getMainCamera()->getWorldScalar(height);
}

LevelSection::~LevelSection() {
  EntityManager* entityManager = _game->getGameplayScreen()->getEntityManager();
  Level* level = _game->getGameplayScreen()->getLevel();

  if(entityManager != nullptr) {
    for(unsigned int k = 0; k < _objects.size(); k++) {
      unsigned int objectId = _objects[k];
      entityManager->deleteEntity(objectId);
      _objects.erase(_objects.begin() + k);
    }
  }
}

void LevelSection::update(float deltaTime) {
  EntityManager* entityManager = _game->getGameplayScreen()->getEntityManager();
  Ess2D::Camera2D* camera = _game->getGameplayScreen()->getMainCamera();
  Level* level = _game->getGameplayScreen()->getLevel();
  Entity* entity;

  for(unsigned int k = 0; k < _objects.size();) {
    entity = entityManager->getEntity(_objects[k]);
    if(entity == nullptr) {
      _objects.erase(_objects.begin() + k);
      continue;
    }

    //if this object had been spawned but went out of viewpot OR if it's health is 0 then we just discard it.
    if((entity->isSpawned() && !entity->inViewport()) || entity->getHealth() <= 0) {
      entity->die();
      entityManager->deleteEntity(_objects[k]);
      _objects.erase(_objects.begin() + k);
      continue;
    }

    //stop once we reach the first object that is not in viewport and it's not spawned. it means it is inactive atm because we haven't gotten there yet, so no reason to update any further since objects are sorted by their y position.
    if(!entity->isSpawned() && !entity->inViewport() && entity->getPosition().y > camera->getPosition().y / camera->getZoom()) {
      break;
    }

    //this entity IS in the viewport but it hasn't been spanwed it. so let's spawn it and set it as active.
    if(!entity->isSpawned()) {
      entity->spawn();
    }

    entity->update(deltaTime);

    k++;
  }
}

void LevelSection::draw(float x, float y) {
  Ess2D::SpriteBatch* spriteBatch = _game->getGameplayScreen()->getSpriteBatch();
  /*
  glm::vec4 position(x, y, _width, _height);
  glm::vec4 uv(0.0f, 0.0f, 1.0f, 1.0f);
  Ess2D::ColorRGBA8 color(255, 255, 255, 255);
  spriteBatch->draw(position, uv, _backgroundId, color, 0);
  */
  Ess2D::Camera2D* camera = _game->getGameplayScreen()->getMainCamera();
  EntityManager* entityManager = _game->getGameplayScreen()->getEntityManager();
  Entity* entity;

  for(unsigned int k = 0; k < _objects.size();) {
    entity = entityManager->getEntity(_objects[k]);
    if(entity == nullptr) {
      _objects.erase(_objects.begin() + k);
      continue;
    }

    if(!entity->isSpawned() && !entity->inViewport()) {
      break;
    }

    entity->draw();

    k++;
  }
}

void LevelSection::addObject(unsigned int objectId) {
  _objects.push_back(objectId);
  std::stable_sort(_objects.begin(), _objects.end(), compareObjectYPosition);
}

bool LevelSection::compareObjectYPosition(unsigned int a, unsigned int b) {
  EntityManager* entityManager = Game::GetInstance()->getGameplayScreen()->getEntityManager();
  return (entityManager->getEntity(a)->getPosition().y < entityManager->getEntity(b)->getPosition().y);
}