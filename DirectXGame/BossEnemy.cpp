#include "BossEnemy.h"
#include "Function.h"
#include "GameScene.h"
#include "imgui.h"
#include <cassert>

BossEnemy::~BossEnemy() {}

void BossEnemy::Initialize(Model* model, uint32_t textureHandle, const Vector3 pos) {
	// NULLポインタチェック
	assert(model);

	model_ = model;
	textureHandle_ = textureHandle;

	worldTransform_.Initialize();

	worldTransform_.translation_ = pos;

	worldTransform_.scale_ = {10, 10, 10};
}

void BossEnemy::Update() { worldTransform_.UpdateMatrix(); }

void BossEnemy::Draw(const ViewProjection& viewProjection) {
	// 3Dモデルを描画
	model_->Draw(worldTransform_, viewProjection, textureHandle_);
}

Vector3 BossEnemy::GetWorldPosition() {

	// ワールド座標を入れる変数
	Vector3 worldPos;
	// ワールド行列の並行移動成分を取得（ワールド座標）
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

void BossEnemy::OnCollision() { isDead_ = true; }
