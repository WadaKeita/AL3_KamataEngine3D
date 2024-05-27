#pragma once
#include "WorldTransform.h"
#include "ViewProjection.h"

class RailCamera {

public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(const Vector3& position, const Vector3& rotate);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	const ViewProjection& GetViewProjection() { return viewProjection_; }
	const WorldTransform& GetWorldTransform() { return worldTransform_; }

	private:
		// ワールド変換行列
	    WorldTransform worldTransform_;
		// ビュープロジェクション
	    ViewProjection viewProjection_;
};
