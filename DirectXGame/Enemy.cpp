#include "Enemy.h"
#include "Function.h"
#include <cassert>

void Enemy::Initialize(Model* model, uint32_t textureHandle) {
	// NULLポインタチェック
	assert(model);

	model_ = model;
	textureHandle_ = textureHandle;

	worldTransform_.Initialize();

	worldTransform_.translation_ = {0, 2, 30};
}

void Enemy::Update() {
	worldTransform_.UpdateMatrix();

	// キャラクターの移動ベクトル
	Vector3 move = {0, 0, 0};

	// 移動速度
	const float kCharacterSpeed = 0.2f;
	
	move.z -= kCharacterSpeed;

	// 座標移動
	worldTransform_.translation_ = Add(worldTransform_.translation_, move);
}

void Enemy::Draw(ViewProjection& viewProjection) {

	// 3Dモデルを描画
	model_->Draw(worldTransform_, viewProjection, textureHandle_);
}
