#include "PlayerMessage.h"

#include "FileSystem.h"
#include "Sound.h"
#include "UI/MessageDialog.h"
#include "UI/Tooltip.h"
#include "UI/Window.h"

#include "Data/CityInfo.h"
#include "Data/Language.h"
#include "Data/Message.h"

#include <string.h>

static int getNewMessageId();

// TODO add setter for outside
static int playSound = 1;
static int consecutiveMessageDelay;

static int hasVideo(int textId)
{
	int offset = Data_Language_Message.index[textId].videoLinkOffset;
	if (!offset) {
		return 0;
	}
	const char *videoFile = &Data_Language_Message.data[offset];
	return FileSystem_fileExists(videoFile);
}

void PlayerMessage_post(int usePopup, int messageType, int param1, short param2)
{
	int id = getNewMessageId();
	if (id < 0) {
		return;
	}
	Data_Message.totalMessages++;
	Data_Message.currentMessageId = id;

	struct Data_PlayerMessage *m = &Data_Message.messages[id];
	m->messageType = messageType;
	m->year = Data_CityInfo_Extra.gameTimeYear;
	m->month = Data_CityInfo_Extra.gameTimeMonth;
	m->param1 = param1;
	m->param2 = param2;
	m->sequence = Data_Message.nextMessageSequence++;
	int textId = PlayerMessage_getMessageTextId(messageType);
	int langMessageType = Data_Language_Message.index[textId].messageType;
	if (langMessageType == MessageType_Disaster || langMessageType == MessageType_Invasion) {
		Data_Message.hotspotCount = 1;
		UI_Window_requestRefresh();
	}
	if (usePopup && UI_Window_getId() == Window_City) {
		consecutiveMessageDelay = 5;
		m->readFlag = 1;
		UI_Tooltip_resetTimer();
		if (!hasVideo(textId)) {
			if (Data_Language_Message.index[textId].isUrgent == 1) {
				Sound_Effects_playChannel(SoundChannel_FanfareUrgent);
			} else {
				Sound_Effects_playChannel(SoundChannel_Fanfare);
			}
		}
		UI_MessageDialog_setPlayerMessage(
			m->year, m->month, m->param1, m->param2,
			PlayerMessage_getAdvisorForMessageType(m->messageType), usePopup);
		UI_MessageDialog_show(textId, 0);
	} else if (usePopup) {
		// add to queue to be processed when player returns to city
		for (int i = 0; i < 20; i++) {
			if (!Data_Message.popupMessageQueue[i]) {
				Data_Message.popupMessageQueue[i] = m->sequence;
				break;
			}
		}
	} else if (playSound) {
		if (Data_Language_Message.index[textId].isUrgent == 1) {
			Sound_Effects_playChannel(SoundChannel_FanfareUrgent);
		} else {
			Sound_Effects_playChannel(SoundChannel_Fanfare);
		}
	}
	playSound = 1;
}

void PlayerMessage_processQueue()
{
	if (UI_Window_getId() != Window_City) {
		return;
	}
	if (consecutiveMessageDelay > 0) {
		consecutiveMessageDelay--;
		return;
	}
	int messageSeq = 0;
	for (int i = 0; i < 20; i++) {
		if (Data_Message.popupMessageQueue[i]) {
			messageSeq = Data_Message.popupMessageQueue[i];
			Data_Message.popupMessageQueue[i] = 0;
			break;
		}
	}
	if (messageSeq == 0) {
		return;
	}
	int msgId = -1;
	for (int i = 0; i < 999; i++) {
		if (!Data_Message.messages[i].messageType) {
			return;
		}
		if (Data_Message.messages[i].sequence == messageSeq) {
			msgId = i;
			break;
		}
	}
	if (msgId < 0) {
		return;
	}
	consecutiveMessageDelay = 5;
	struct Data_PlayerMessage *m = &Data_Message.messages[msgId];
	m->readFlag = 1;
	int textId = PlayerMessage_getMessageTextId(m->messageType);
	UI_Tooltip_resetTimer();
	if (!hasVideo(textId)) {
		if (Data_Language_Message.index[textId].isUrgent == 1) {
			Sound_Effects_playChannel(SoundChannel_FanfareUrgent);
		} else {
			Sound_Effects_playChannel(SoundChannel_Fanfare);
		}
	}
	UI_MessageDialog_setPlayerMessage(
		m->year, m->month, m->param1, m->param2,
		PlayerMessage_getAdvisorForMessageType(m->messageType), 1);
	UI_MessageDialog_show(textId, 0);
}

static int getNewMessageId()
{
	for (int i = 0; i < MAX_MESSAGES; i++) {
		if (!Data_Message.messages[i].messageType) {
			return i;
		}
	}
	return -1;
}

int PlayerMessage_getMessageTextId(int messageType)
{
	if (messageType > 50) {
		return messageType + 199;
	} else {
		return messageType + 99;
	}
}

int PlayerMessage_getAdvisorForMessageType(int messageType)
{
	switch (messageType) {
		case 22:
		case 23:
		case 24:
		case 25:
		case 26:
		case 27:
			return MessageAdvisor_Military;
		case 28:
		case 29:
		case 30:
		case 31:
		case 32:
		case 33:
		case 34:
		case 35:
			return MessageAdvisor_Imperial;
		case 36:
		case 37:
			return MessageAdvisor_Labor;
		case 49:
		case 50:
			return MessageAdvisor_Population;
		case 102:
		case 103:
		case 104:
			return MessageAdvisor_Health;
		default:
			return MessageAdvisor_None;
	}
}

void PlayerMessage_initList()
{
	for (int i = 0; i < MAX_MESSAGES; i++) {
		Data_Message.messages[i].messageType = 0;
	}
	Data_Message.populationMessagesShown.pop0 = 0;
	Data_Message.populationMessagesShown.pop500 = 0;
	Data_Message.populationMessagesShown.pop1000 = 0;
	Data_Message.populationMessagesShown.pop2000 = 0;
	Data_Message.populationMessagesShown.pop3000 = 0;
	Data_Message.populationMessagesShown.pop5000 = 0;
	Data_Message.populationMessagesShown.pop10000 = 0;
	Data_Message.populationMessagesShown.pop15000 = 0;
	Data_Message.populationMessagesShown.pop20000 = 0;
	Data_Message.populationMessagesShown.pop25000 = 0;
	for (int i = 0; i < 20; i++) {
		Data_Message.messageCategoryCount[i] = 0;
		Data_Message.messageDelay[i] = 0;
	}
	for (int i = 0; i < 20; i++) {
		Data_Message.popupMessageQueue[i] = 0;
	}
	Data_Message.consecutiveMessageDelay = 0;
	Data_Message.nextMessageSequence = 0;
	Data_Message.totalMessages = 0;
	Data_Message.currentMessageId = -1;
	Data_Message.currentProblemAreaMessageId = 0;

	Data_Message.scrollPosition = 0;
	Data_Message.maxScrollPosition = 0;

	Data_Message.playSound = 1;
	Data_Message.lastSoundTime.fire = 0;
	Data_Message.lastSoundTime.collapse = 0;
	Data_Message.lastSoundTime.rioterGenerated = 0;
	Data_Message.lastSoundTime.rioterCollapse = 0;

	PlayerMessage_initProblemArea();
}

void PlayerMessage_initProblemArea()
{
	// TODO
}

void PlayerMessage_sortMessages()
{
	struct Data_PlayerMessage tmpMessage;
	for (int i = 0; i < MAX_MESSAGES; i++) {
		for (int a = 0; a < MAX_MESSAGES - 1; a++) {
			int swap = 0;
			if (Data_Message.messages[a].messageType) {
				if (Data_Message.messages[a].sequence < Data_Message.messages[a+1].sequence) {
					if (Data_Message.messages[a+1].messageType) {
						swap = 1;
					}
				}
			} else if (Data_Message.messages[a+1].messageType) {
				swap = 1;
			}
			if (swap) {
				memcpy(&tmpMessage, &Data_Message.messages[a], sizeof(struct Data_PlayerMessage));
				memcpy(&Data_Message.messages[a], &Data_Message.messages[a+1], sizeof(struct Data_PlayerMessage));
				memcpy(&Data_Message.messages[a+1], &tmpMessage, sizeof(struct Data_PlayerMessage));
			}
		}
	}
	Data_Message.totalMessages = 0;
	for (int i = 0; i < MAX_MESSAGES; i++) {
		if (Data_Message.messages[i].messageType) {
			Data_Message.totalMessages++;
		}
	}
	if (Data_Message.totalMessages <= 10) {
		Data_Message.scrollPosition = 0;
	} else {
		Data_Message.maxScrollPosition = Data_Message.totalMessages - 10;
	}
}

void PlayerMessage_updateMessageDelays()
{
	for (int i = 0; i < 20; i++) {
		if (Data_Message.messageDelay[i] > 0) {
			Data_Message.messageDelay[i]--;
		}
	}
}

