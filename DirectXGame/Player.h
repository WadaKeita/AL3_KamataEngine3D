#pragma once
#include "Input.h"
#include "Model.h"
#include "PlayerBullet.h"
#include "Sprite.h"
#include "WorldTransform.h"
#include "ViewProjection.h"
#include <list>

/// <summary>
/// 自キャラ
/// </summary>
class Player {
public:
	/// <summary>
	/// デストラクタ
	/// </summary>
	~Player();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model">モデル</param>
	/// <param name="textureHandle">テクスチャハンドル</param>
	void Initialize(Model* model, uint32_t textureHandle, const Vector3& position);

	/// <summary>
	/// 更新
	/// </summary>
	void Update(const ViewProjection& viewProjection);

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="viewProjection">ビュープロジェクション（参照渡し）</param>
	void Draw3D(const ViewProjection& viewProjection);

	/// <summary>
	/// 回転
	/// </summary>
	void Rotate();

	/// <summary>
	/// 攻撃
	/// </summary>
	void Attack();

	// ワールド座標を取得
	Vector3 GetWorldPosition();

	Vector3 GetWorldPosition3DReticle();

	// 衝突を検出したら呼び出されるコールバック関数
	void OnCollision();

	// 弾リストを取得
	const std::list<PlayerBullet*>& GetBullets() const { return bullets_; }

	// 半径を取得
	const int GetRadius() const { return radius_; };

	/// <summary>
	/// 親となるワールドトランスフォームをセット
	/// </summary>
	/// <param name="parent"></param>
	void SetParent(const WorldTransform* parent);

	/// <summary>
	/// UI描画
	/// </summary>
	void DrawUI();

private:
	// ワールド変換データ
	WorldTransform worldTransform_;
	ViewProjection viewProjection_;
	// モデル
	Model* model_ = nullptr;
	// テクスチャハンドル
	uint32_t textureHandle_ = 0u;

	// キーボード入力
	Input* input_ = nullptr;

	// 弾
	std::list<PlayerBullet*> bullets_;

	// 半径
	const int radius_ = 1;

	// 3Dレティクル用ワールドトランスフォーム
	WorldTransform worldTransform3DReticle_;

	// 2Dレティクル用スプライト
	Sprite* sprite2DReticle_ = nullptr;
};
