#pragma once

#include <math.h>
#include <glm\glm.hpp>
#include <string>

class ProjectileSpawner {
  public:
    ProjectileSpawner();
    ProjectileSpawner(float fireRate, glm::vec2 size, float projectileDamage);
    ~ProjectileSpawner();

    void setFireRate(float fireRate) { _fireRate = fireRate; }
    void setSource(int source) { _source = source; }
    int update(float deltaTime, bool isFiring, const glm::vec2& position, const glm::vec2& velocity, float angle);
    void spawn(int projectileCount, const glm::vec2& position, const glm::vec2& velocity, float angle);
    void setSkin(const std::string& skinName);
    void setVelocity(float velocity);
    void setPlaySound(bool playSound);

    float getProjectileWidth() { return _projectileWidth; }
    float getProjectileHeight() { return _projectileHeight; }

  private:
    float _fireRate = 0.3f;
    float _fireRateCounter = 1.0f;

    std::string _projectileSkin;
    float _projectileVelocity = 2.0f;
    float _projectileWidth = 5.0f;
    float _projectileHeight = 5.0f;
    float _projectileDamage = 10.0f;
    int _source = 2;

    bool _playSound = false;


};

