#include "Player.h"
#include "Utils.h"
#include <iostream>

Player::Player() : Player(0, glm::vec4(0.0f), 0.0f, 0.0f, glm::vec2(0.0f, 0.0f)) { }

Player::Player(int textureId, glm::vec4 uv, float width, float height, glm::vec2 position) : Entity(textureId, uv, width, height, position) {
  _projectileSpawnerLeft = ProjectileSpawner(_fireRate, glm::vec2(0.3f, 0.7f), 40.0f);
  _projectileSpawnerLeft.setSource(this->getType());
  _projectileSpawnerLeft.setPlaySound(true);
  _projectileSpawnerLeft.setSkin("bullet_orange");
  _projectileSpawnerLeft.setVelocity(40.0f);
  _projectileSpawnerRight = ProjectileSpawner(_fireRate, glm::vec2(0.3f, 0.7f), 40.0f);
  _projectileSpawnerRight.setSource(this->getType());
  _projectileSpawnerRight.setPlaySound(true);
  _projectileSpawnerRight.setSkin("bullet_orange");
  _projectileSpawnerRight.setVelocity(40.0f);

  // SPACESHIP
  Ess2D::TextureAtlas * playerAtlas = _game->getGameplayScreen()->getTextureCache()->getAtlas("Textures/player.png", "Textures/player.json");

  _animationManager = new Ess2D::AnimationManager();
  
  Ess2D::Animation* idleAnimation = _animationManager->add("IDLE");
  idleAnimation->setPlaybackRate(10.0f / 60.0f);
  idleAnimation->setTextureAtlas(playerAtlas);
  idleAnimation->setFrames(std::vector<std::string> {"Spaceship_default"});

  Ess2D::Animation* bankLeftAnimation = _animationManager->add("BANK_LEFT");
  bankLeftAnimation->setPlaybackRate(2.5f / 60.0f);
  bankLeftAnimation->setTextureAtlas(playerAtlas);
  bankLeftAnimation->setRepeat(false);
  bankLeftAnimation->setFrames(std::vector<std::string> {"Spaceship_left01", "Spaceship_left02", "Spaceship_left03"});

  Ess2D::Animation* bankRightAnimation = _animationManager->add("BANK_RIGHT");
  bankRightAnimation->setPlaybackRate(2.5f / 60.f);
  bankRightAnimation->setTextureAtlas(playerAtlas);
  bankRightAnimation->setRepeat(false);
  bankRightAnimation->setFrames(std::vector<std::string> {"Spaceship_right01", "Spaceship_right02", "Spaceship_right03"});

  _animationManager->play("IDLE");

  glm::vec2 frameSize = idleAnimation->getTextureAtlas()->getSize(idleAnimation->getCurrentFrame());

  _horizontalScaleFactor = _width / frameSize.x;

  // THRUSTER

  Ess2D::TextureAtlas * thrusterAtlas = _game->getGameplayScreen()->getTextureCache()->getAtlas("Textures/thruster.png", "Textures/thruster.json");

  _thrusterAnimationManager = new Ess2D::AnimationManager();

  Ess2D::Animation* thrusterIdleAnim = _thrusterAnimationManager->add("IDLE");
  thrusterIdleAnim->setPlaybackRate(1.0f / 60.0f);
  thrusterIdleAnim->setTextureAtlas(thrusterAtlas);
  std::vector<std::string> thrusterAnimationFrames;
  for (int i = 0; i < 36; i++) {
    std::string extraZero = "0";
    if (i > 8) {
      extraZero = "";
    }
    thrusterAnimationFrames.push_back("Thruster_" + extraZero + std::to_string(i + 1));
  }
  
  thrusterIdleAnim->setFrames(thrusterAnimationFrames);

  _thrusterAnimationManager->play("IDLE");

  _thrusterWidth = _game->getGameplayScreen()->getMainCamera()->getWorldScalar(_thrusterWidth);
  _thrusterHeight = _game->getGameplayScreen()->getMainCamera()->getWorldScalar(_thrusterHeight);

  //MUZZLE FLASH
  Ess2D::TextureAtlas * muzzleAtlas = _game->getGameplayScreen()->getTextureCache()->getAtlas("Textures/muzzle.png", "Textures/muzzle.json");

  _muzzleLeftAnimationManager = new Ess2D::AnimationManager();
  _muzzleRightAnimationManager = new Ess2D::AnimationManager();

  std::vector<std::string> muzzleAnimationFrames;
  for (int i = 0; i <= 16; i++) {
    muzzleAnimationFrames.push_back("muzzle_1_" + std::to_string(i));
  }

  Ess2D::Animation* muzzleLeftAnimation = _muzzleLeftAnimationManager->add("MUZZLE");
  muzzleLeftAnimation->setPlaybackRate(0.8f / 60.0f);
  muzzleLeftAnimation->setTextureAtlas(muzzleAtlas);
  muzzleLeftAnimation->setFrames(muzzleAnimationFrames);
  muzzleLeftAnimation->setRepeat(false);
  muzzleLeftAnimation->setAutoStop(true);

  Ess2D::Animation* muzzleRightAnimation = _muzzleRightAnimationManager->add("MUZZLE");
  muzzleRightAnimation->setPlaybackRate(0.8f / 60.0f);
  muzzleRightAnimation->setTextureAtlas(muzzleAtlas);
  muzzleRightAnimation->setFrames(muzzleAnimationFrames);
  muzzleRightAnimation->setRepeat(false);
  muzzleRightAnimation->setAutoStop(true);
}

Player::~Player() { 
  delete _animationManager;
  delete _thrusterAnimationManager;
  delete _muzzleLeftAnimationManager;
  delete _muzzleRightAnimationManager;
}

bool Player::update(float deltaTime) {
  b2Vec2 velocity = _body->GetLinearVelocity();
  b2Vec2 force(0.0f, 0.0f), acceleration(0.0f, 0.0f), desiredVelocity(0.0f, 0.0f);
  float maxSpeed = b2Max(_maxVelocity.x, _maxVelocity.y);

  //Determine animations
  if (_direction.x < 0) {
    _animationManager->play("BANK_LEFT");
  } else if (_direction.x > 0) {
    _animationManager->play("BANK_RIGHT");
  } else {
    if (_animationManager->getCurrentAnimationName() != "IDLE") {
      Ess2D::Animation* currentAnimation = _animationManager->getCurrent();
      if (!currentAnimation->isReversed()) {
        currentAnimation->setReverse(true);
      } else if (currentAnimation->getCurrentFrameNumber() == 0) {
        _animationManager->play("IDLE");
      }
    }
  }

  //Handle Physics
  if(_direction.x != 0) {
    acceleration.x = (_direction.x * _maxVelocity.x - velocity.x);
  } else if(velocity.x != 0) {
    acceleration.x = (_defaultVelocity.x - velocity.x);
  }

  if(_direction.y != 0) {
    if(_direction.y < 0) {
      acceleration.y = _direction.y * _maxVelocity.y - velocity.y;
    } else {
      acceleration.y = (b2Min(_direction.y * (_maxVelocity.y + _defaultVelocity.y), velocity.y + 8.5f) - velocity.y);
    }
  } else {
    acceleration.y = (_defaultVelocity.y - velocity.y);
  }

  force = _body->GetMass() * acceleration;

  _body->ApplyLinearImpulse(force, _body->GetWorldCenter(), true);

  float currentSpeed = velocity.Length();
  if(currentSpeed > maxSpeed && _direction.y != 0) {
    _body->SetLinearVelocity((maxSpeed / currentSpeed) * velocity);
  }

  /* BIND PLAYER WITHIN THE VIEWPORT */
  //calculate next step position
  glm::vec2 nextPosition = glm::vec2(
    _body->GetPosition().x + _body->GetLinearVelocity().x * deltaTime + (acceleration.x * deltaTime * deltaTime) / 2,
    _body->GetPosition().y + _body->GetLinearVelocity().y * deltaTime + (acceleration.y * deltaTime * deltaTime) / 2
  );

  glm::vec2 viewportSize = _game->getGameplayScreen()->getMainCamera()->getWorldViewportSize();
  glm::vec2 cameraPosition = _game->getGameplayScreen()->getMainCamera()->getPosition() / _game->getGameplayScreen()->getMainCamera()->getZoom();
  b2Vec2 correctedPosition = _body->GetPosition();
  b2Vec2 correctionAcceleration = b2Vec2(0.0f, 0.0f);
  b2Vec2 currentVelocity = _body->GetLinearVelocity();

  bool doCorrectPosition = false;

  if(nextPosition.x - _width / 2 < cameraPosition.x - viewportSize.x / 2) {
    correctedPosition.x = cameraPosition.x - viewportSize.x / 2 + _width / 2;
    correctionAcceleration.x = 0.0f - currentVelocity.x;
    doCorrectPosition = true;
  }
  if(nextPosition.x + _width / 2 > cameraPosition.x + viewportSize.x / 2) {
    correctedPosition.x = cameraPosition.x + viewportSize.x / 2 - _width / 2;
    correctionAcceleration.x = 0.0f - currentVelocity.x;
    doCorrectPosition = true;
  }
  if(nextPosition.y - _height / 2 < cameraPosition.y - viewportSize.y / 2 && _direction.y != 1) {
    correctedPosition.y = cameraPosition.y - viewportSize.y / 2 + _height / 2;
    correctionAcceleration.y = 0.0f - currentVelocity.y;
    doCorrectPosition = true;
  }
  if(nextPosition.y + _height / 2 > cameraPosition.y + viewportSize.y / 2 && _direction.y != -1) {
    correctedPosition.y = cameraPosition.y + viewportSize.y / 2 - _height / 2;
    correctionAcceleration.y = _defaultVelocity.y * 0.99f - currentVelocity.y;
    doCorrectPosition = true;
  }
  
  //if we have corrections to do, we must make sure to stop the body's velocity in the corrected direction as well.
  //still can be a bit weird... could be interpolated camera position or something...
  if(doCorrectPosition) {
    b2Vec2 force = _body->GetMass() * correctionAcceleration;
    //the impulse is applied in order to stop the body from moving further in that direction.
    _body->ApplyLinearImpulse(force, _body->GetWorldCenter(), true);

    _body->SetTransform(correctedPosition, _body->GetAngle());
  }

  //Update Projectile Spawners
  _projectileSpawnerLeftPosition = glm::vec2(-_width / 2 + 0.5f, 0.1f);
  _projectileSpawnerRightPosition = _projectileSpawnerLeftPosition + glm::vec2(_width - 1.0f, 0.0f);

  correctProjectileSpawnersPosition(_animationManager->getCurrent()->getCurrentFrame());

  int projectilesSpawnedLeft = _projectileSpawnerLeft.update(deltaTime, _isFiring, Utils::toVec2(_body->GetPosition()) + _projectileSpawnerLeftPosition + glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 1.0f), _body->GetAngle());
  int projectilesSpawnedRight = _projectileSpawnerRight.update(deltaTime, _isFiring, Utils::toVec2(_body->GetPosition()) + _projectileSpawnerRightPosition + glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 1.0f), _body->GetAngle());

  _game->getGameplayScreen()->addShotsFired(projectilesSpawnedLeft + projectilesSpawnedRight);

  if(projectilesSpawnedLeft > 0) {
    _muzzleLeftAnimationManager->play("MUZZLE");
    _muzzleLeftAnimationManager->getCurrent()->reset();
    _muzzleRightAnimationManager->play("MUZZLE");
    _muzzleRightAnimationManager->getCurrent()->reset();
  }

  //Update Animations
  _animationManager->update(deltaTime);
  _thrusterAnimationManager->update(deltaTime);
  _muzzleLeftAnimationManager->update(deltaTime);
  _muzzleRightAnimationManager->update(deltaTime);

  return true;
}

void Player::draw() {
  if(_isSpawned) {
    b2Vec2 bodyPosition = this->_body->GetPosition();
    glm::vec2 screenPosition = _position;
    
    Ess2D::SpriteBatch* spriteBatch = _game->getGameplayScreen()->getSpriteBatch();
    Ess2D::TextureAtlas* textureAtlas = _animationManager->getCurrent()->getTextureAtlas();
    std::string currentAnimationFrame = _animationManager->getCurrent()->getCurrentFrame();

    float width = textureAtlas->getSize(currentAnimationFrame).x * _horizontalScaleFactor;

    spriteBatch->draw(glm::vec4(screenPosition.x - width / 2, screenPosition.y - _height / 2, width, _height), textureAtlas->getUV(currentAnimationFrame), textureAtlas->getTextureId(), _color, (float)_depth, _body->GetAngle());

    Ess2D::TextureAtlas* thrusterTextureAtlas = _thrusterAnimationManager->getCurrent()->getTextureAtlas();
    std::string thrusterCurrentAnimationFrame = _thrusterAnimationManager->getCurrent()->getCurrentFrame();

    spriteBatch->draw(glm::vec4(screenPosition.x - _thrusterWidth / 2, screenPosition.y - _height + 0.53f, _thrusterWidth, _thrusterHeight), thrusterTextureAtlas->getUV(thrusterCurrentAnimationFrame), thrusterTextureAtlas->getTextureId(), _color, (float)_depth - 1);

    if(_muzzleLeftAnimationManager->isPlaying()) {
      Ess2D::TextureAtlas* muzzleLeftTextureAtlas = _muzzleLeftAnimationManager->getCurrent()->getTextureAtlas();
      std::string muzzleLeftCurrentAnimationFrame = _muzzleLeftAnimationManager->getCurrent()->getCurrentFrame();

      glm::vec2 leftMuzzlePosition = screenPosition + _projectileSpawnerLeftPosition + glm::vec2(-0.35f, -0.05f);
      spriteBatch->draw(glm::vec4(leftMuzzlePosition, 0.9f, 0.9f), muzzleLeftTextureAtlas->getUV(muzzleLeftCurrentAnimationFrame), muzzleLeftTextureAtlas->getTextureId(), _color, (float)_depth - 1);

      Ess2D::TextureAtlas* muzzleRightTextureAtlas = _muzzleRightAnimationManager->getCurrent()->getTextureAtlas();
      std::string muzzleRightCurrentAnimationFrame = _muzzleRightAnimationManager->getCurrent()->getCurrentFrame();

      glm::vec2 rightMuzzlePosition = screenPosition + _projectileSpawnerRightPosition + glm::vec2(-0.40f, -0.05f);
      spriteBatch->draw(glm::vec4(rightMuzzlePosition, 0.9f, 0.9f), muzzleRightTextureAtlas->getUV(muzzleRightCurrentAnimationFrame), muzzleRightTextureAtlas->getTextureId(), _color, (float)_depth - 1);
    }
  }
}

void Player::createFixtures() {
  b2Vec2 vertices[3];
  vertices[0].Set(-_width / 4.5f, 0.0f);
  vertices[1].Set(_width / 4.5f, 0.0f);
  vertices[2].Set(0.0f, _height / 2);
  int32 count = 3;

  b2PolygonShape boxShape;
  boxShape.Set(vertices, count);

  //boxShape.SetAsBox(_width / 2, _height / 2);

  b2FixtureDef boxFixtureDef;
  boxFixtureDef.shape = &boxShape;
  boxFixtureDef.density = 1;
  boxFixtureDef.isSensor = 1;

  _body->CreateFixture(&boxFixtureDef);

  b2PolygonShape boxShape2;
  boxShape2.SetAsBox(_width / 2.5f, _height / 4, b2Vec2(0.0f, -_height / 4.6f), 0);

  b2FixtureDef boxFixtureDef2;
  boxFixtureDef2.shape = &boxShape2;
  boxFixtureDef2.density = 1;
  boxFixtureDef2.isSensor = 1;
  _body->CreateFixture(&boxFixtureDef2);
}

void Player::correctProjectileSpawnersPosition(const std::string& currentPlayerFrame) {
  if(currentPlayerFrame == "Spaceship_left01") {
    _projectileSpawnerLeftPosition += glm::vec2(0.1f, 0.0f);
  } else if(currentPlayerFrame == "Spaceship_left02") {
    _projectileSpawnerLeftPosition += glm::vec2(0.25f, 0.0f);
    _projectileSpawnerRightPosition += glm::vec2(-0.05f, 0.0f);
  } else if(currentPlayerFrame == "Spaceship_left03") {
    _projectileSpawnerLeftPosition += glm::vec2(0.45f, 0.0f);
    _projectileSpawnerRightPosition += glm::vec2(-0.17f, 0.0f);
  } else if(currentPlayerFrame == "Spaceship_right01") {
    _projectileSpawnerRightPosition += glm::vec2(-0.1f, 0.0f);
  } else if(currentPlayerFrame == "Spaceship_right02") {
    _projectileSpawnerRightPosition += glm::vec2(-0.25f, 0.0f);
    _projectileSpawnerLeftPosition += glm::vec2(0.05f, 0.0f);
  } else if(currentPlayerFrame == "Spaceship_right03") {
    _projectileSpawnerRightPosition += glm::vec2(-0.45f, 0.0f);
    _projectileSpawnerLeftPosition += glm::vec2(0.17f, 0.0f);
  }
}

void Player::contact(Entity* e) {
  //not exactly elegant, but oh well.
  if(e->getType() == ET_ENTITY || e->getType() == ET_SPACESHIP_A || e->getType() == ET_SPACESHIP_B || e->getType() == ET_SPACESHIP_C || e->getType() == ET_SPACESHIP_D) {
    if(e->isDead()) { //if it's dead, we don't care about this collision
      return;
    }
    
    applyDamage(e->getCollisionDamage()); //apply collision damage to player
    e->setHealth(0); //kill entity

    _game->getGameplayScreen()->addScore(50);
    _game->getGameplayScreen()->addEnemyShot();

    _game->getGameplayScreen()->getEntityManager()->deleteEntity(e->getId(), true); //schedule deletion
  }

  if(e->getType() == ET_POWERUP) {
    Powerup* powerup = static_cast <Powerup*> (e);
    std::string powerupType = powerup->getPowerupType();

    if(powerupType == "hp") {
      _game->getGameplayScreen()->addHealth(100);
    } else if(powerupType == "score") {
      _game->getGameplayScreen()->addScore(50);
    } else if(powerupType == "dmg") {
      applyDamage(50);
    }

    powerup->setHealth(0);
  }
}

