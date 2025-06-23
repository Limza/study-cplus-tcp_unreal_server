#pragma once
class Object : public std::enable_shared_from_this<Object>
{
public:
	Object();
	virtual ~Object();
	NON_COPYABLE_CLASS(Object);

public:
	[[nodiscard]] bool IsPlayer() const { return _isPlayer; }

public:
	Protocol::ObjectInfo* objectInfo;
	Protocol::PosInfo* posInfo;
	std::atomic<std::weak_ptr<Room>> room;

protected:
	bool _isPlayer = false;
};

