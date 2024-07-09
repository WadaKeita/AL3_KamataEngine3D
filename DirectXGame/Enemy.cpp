#include "Enemy.h"
#include "Function.h"
#include "GameScene.h"
#include "Player.h"
#include "imgui.h"
#include <cassert>

Enemy::~Enemy() {}

void Enemy::Initialize(Model* model, uint32_t textureHandle, const Vector3 pos) {

	// NULLポインタチェック
	assert(model);

	model_ = model;
	textureHandle_ = textureHandle;

	worldTransform_.Initialize();

	worldTransform_.translation_ = pos;

	//InitializeApproach();
}

void Enemy::Update() {

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
	const float kCharacterSpeed = 0.1f;

	move.z -= kCharacterSpeed;

	// 移動（ベクトルを加算）
	worldTransform_.translation_ = Add(worldTransform_.translation_, move);
	//// 規定の位置に到達したら離脱
	//if (worldTransform_.translation_.z < 0.0f) {
	//	phase_ = Phase::Leave;
	//}

	//// 発射タイマーをカウントダウン
	// fireTimer_--;
	//// 指定時間に達した
	// if (fireTimer_ <= 0) {
	//	// 弾を発射
	//	Fire();
	//	// 発射タイマーの初期化
	//	fireTimer_ = kFireInterval;
	// }
}
//
//void Enemy::InitializeApproach() {
//	// 発射タイマーを初期化
//	fireTimer_ = kFireInterval;
//}

// void Enemy::InitializeLeave() {}
//
// void Enemy::Leave() {
//
//	// キャラクターの移動ベクトル
//	Vector3 move = {0, 0, 0};
//
//	// 移動速度
//	const float kCharacterSpeed = 0.01f;
//
//	move.x -= kCharacterSpeed;
//	move.y += kCharacterSpeed;
//
//	// 移動（ベクトルを加算）
//	worldTransform_.translation_ = Add(worldTransform_.translation_, move);
//}

void Enemy::Hold() {

	// キャラクターの移動ベクトル
	Vector3 move = {0, 0, 0};

	// プレイヤーの現在位置と1f前の位置の差分を元に移動
	move = player_->GetMovePos();

	// 移動（ベクトルを加算）
	worldTransform_.translation_ = Add(worldTransform_.translation_, move);


}

void Enemy::Fire() {
	// 弾があれば解放する(listを使うので必要なし)

	assert(player_);

	// 弾の速度
	const float kBulletSpeed = 1.0f;

	// 敵キャラ->自キャラへの差分ベクトルを求める
	Vector3 endPos = player_->GetWorldPosition();
	Vector3 startPos = GetWorldPosition();

	// 弾の速度を求める
	Vector3 velocity = Multiply(kBulletSpeed, Normalize(Subtract(endPos, startPos)));

	//// 速度ベクトルを敵機の向きに合わせて回転させる
	velocity = TransformNormal(velocity, worldTransform_.matWorld_);

	// 弾を生成し、初期化
	EnemyBullet* newBullet = new EnemyBullet();
	newBullet->Initialize(model_, worldTransform_.translation_, velocity);

	// ゲームシーンに弾を登録する
	gameScene_->AddEnemyBullet(newBullet);
}

void Enemy::Thrown() {

	// キャラクターの移動ベクトル
	Vector3 move = {0, 0, 0};

	// 移動速度
	const float kCharacterSpeed = 0.5f;

	move.z += kCharacterSpeed;

	// 移動（ベクトルを加算）
	worldTransform_.translation_ = Add(worldTransform_.translation_, move);
}

Vector3 Enemy::GetWorldPosition() {

	// ワールド座標を入れる変数
	Vector3 worldPos;
	// ワールド行列の並行移動成分を取得（ワールド座標）
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

void Enemy::OnCollision() {

	if (player_->GetIsCatch()) { return; }
	
	Vector3 worldPos;
	worldPos = GetWorldPosition();

	Vector3 enemyPos = Transform(worldPos, Inverse(railCamera_->GetWorldTransform().matWorld_));

	// エネミーとレールカメラの親子関係を結ぶ
	this->SetParent(&railCamera_->GetWorldTransform());

	worldTransform_.translation_ = enemyPos;

	isHold_ = true;

	phase_ = Phase::Hold;
	player_->SetIsCatch(true);
	player_->SetHoldEnemy(this);
}

void Enemy::OnCollisionEnemy() { isDead_ = true; }

void (Enemy::*Enemy::phaseTable[])() = {&Enemy::Approach, &Enemy::Hold, &Enemy::Thrown};

void Enemy::SetParent(const WorldTransform* parent) {
	// 親子関係を結ぶ
	worldTransform_.parent_ = parent;
}
