#include "pch.h"
#include "ObjectUtils.h"

#include "GameSession.h"
#include "Player.h"

using std::atomic;

atomic<int64> ObjectUtils::s_idGenerator = 1;

PlayerRef ObjectUtils::CreatePlayer(GameSessionRef session)
{
	const int64 newId = s_idGenerator.fetch_add(1);

	auto player = MakeShared<Player>();
	player->objectInfo->set_object_id(newId);
	player->posInfo->set_object_id(newId);

	player->session = session;
	session->player.store(player);

	return player;
}
