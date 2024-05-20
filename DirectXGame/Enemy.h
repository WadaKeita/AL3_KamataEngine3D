#pragma once
#include "EnemyBullet.h"
#include "Model.h"
#include "WorldTransform.h"
#include <list>

// 自機クラスの前方宣言
class Player;

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

	~Enemy();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model">モデル</param>
	/// <param name="textureHandle">テクスチャハンドル</param>
	void Initialize(Model* model, uint32_t textureHandle);

	void Update();

	void Draw(const ViewProjection& viewProjection);

	/// <summary>
	/// 接近フェーズ
	/// </summary>
	void Approach();

	/// <summary>
	/// 接近フェーズ初期化
	/// </summary>
	void InitializeApproach();
	
	/// <summary>
	/// 離脱フェーズ初期化
	/// </summary>
	//void InitializeLeave();
	/// <summary>
	/// 離脱フェーズ
	/// </summary>
	void Leave();

	/// <summary>
	/// 弾発射
	/// </summary>
	void Fire();

	// 発射間隔
	static const int kFireInterval = 60;

	void SetPlayer(Player* player) { player_ = player; }

	// ワールド座標を取得
	Vector3 GetWorldPosition();

	// 衝突を検出したら呼び出されるコールバック関数
	void OnCollision();

	// 弾リストを取得
	const std::list<EnemyBullet*>& GetBullets() const { return bullets_; }
		
	// 半径を取得
	const int GetRadius() const { return radius_; };


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

	// 弾
	std::list<EnemyBullet*> bullets_;

	// 発射タイマー
	int32_t fireTimer_ = 0;

	// 自キャラ
	Player* player_ = nullptr;

	// 半径
	const int radius_ = 1;
};
