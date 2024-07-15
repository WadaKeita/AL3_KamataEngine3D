#pragma once
#include "Model.h"
#include "WorldTransform.h"

class BossEnemy {

public:
public:
	~BossEnemy();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model">モデル</param>
	/// <param name="textureHandle">テクスチャハンドル</param>
	void Initialize(Model* model, uint32_t textureHandle, const Vector3 pos);

	void Update();

	void Draw(const ViewProjection& viewProjection);

	
	// ワールド座標を取得
	Vector3 GetWorldPosition();

	// 衝突を検出したら呼び出されるコールバック関数
	void OnCollision();
	void OnCollisionEnemy();

	void SetGameScene(GameScene* gameScene) { gameScene_ = gameScene; }


	
private:
	// ワールド変換データ
	WorldTransform worldTransform_;
	// モデル
	Model* model_ = nullptr;
	// テクスチャハンドル
	uint32_t textureHandle_ = 0u;
	
	// 発射タイマー
	int32_t fireTimer_ = 0;

	// 半径
	const int radius_ = 1;

	// ゲームシーン
	GameScene* gameScene_ = nullptr;

	// デスフラグ
	bool isDead_ = false;
};
