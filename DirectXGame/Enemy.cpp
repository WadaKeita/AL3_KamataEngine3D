#include "Enemy.h"
#include "Function.h"
#include "imgui.h"
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

	/*switch (phase_) {
	case Phase::Approach:
	default:
		Approach();
		break;

	case Phase::Leave:
		Leave();
		break;
	}*/

	(this->*phaseTable[static_cast<size_t>(phase_)])();


	worldTransform_.UpdateMatrix();

	ImGui::Begin("enemy");

	ImGui::DragFloat3("enemy_pos", &worldTransform_.translation_.x, 0.1f);

	ImGui::End();
}

void Enemy::Draw(const ViewProjection& viewProjection) {

	// 3Dモデルを描画
	model_->Draw(worldTransform_, viewProjection, textureHandle_);
}

void Enemy::Approach() {

	// キャラクターの移動ベクトル
	Vector3 move = {0, 0, 0};

	// 移動速度
	const float kCharacterSpeed = 0.2f;

	move.z -= kCharacterSpeed;

	// 移動（ベクトルを加算）
	worldTransform_.translation_ = Add(worldTransform_.translation_, move);
	// 規定の位置に到達したら離脱
	if (worldTransform_.translation_.z < 0.0f) {
		phase_ = Phase::Leave;
	}
}

void Enemy::Leave() {

	// キャラクターの移動ベクトル
	Vector3 move = {0, 0, 0};

	// 移動速度
	const float kCharacterSpeed = 0.1f;

	move.x -= kCharacterSpeed;
	move.y += kCharacterSpeed;

	// 移動（ベクトルを加算）
	worldTransform_.translation_ = Add(worldTransform_.translation_, move);
}

void (Enemy::*Enemy::phaseTable[])() = {
	&Enemy::Approach,
	&Enemy::Leave
};