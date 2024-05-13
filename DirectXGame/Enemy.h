#pragma once
#include "Model.h"
#include "WorldTransform.h"

// 行動フェーズ
enum class Phase {
	Approach,
	Leave,
};

/// <summary>
/// 敵
/// </summary>
class Enemy {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model">モデル</param>
	/// <param name="textureHandle">テクスチャハンドル</param>
	void Initialize(Model* model, uint32_t textureHandle);

	void Update();

	void Draw(const ViewProjection& viewProjection);

	void Approach();

	void Leave();

private:
	// ワールド変換データ
	WorldTransform worldTransform_;
	// モデル
	Model* model_ = nullptr;
	// テクスチャハンドル
	uint32_t textureHandle_ = 0u;
	
	// フェーズ
	Phase phase_ = Phase::Approach;

	// メンバ関数ポインタのテーブル
	static void (Enemy::*phaseTable[])();
};
