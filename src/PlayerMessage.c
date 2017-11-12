#include "PlayerMessage.h"

#include "CityView.h"
#include "Formation.h"
#include "Sound.h"

#include "UI/MessageDialog.h"
#include "UI/Tooltip.h"
#include "UI/Window.h"

#include "Data/CityInfo.h"
#include "Data/Message.h"

#include "city/message.h"
#include "core/file.h"
#include "core/lang.h"
#include "core/time.h"
#include "game/time.h"

#include <string.h>

static int getNewMessageId();

static int playSound = 1;
static int consecutiveMessageDelay;

static int hasVideo(int textId)
{
    const lang_message *msg = lang_get_message(textId);
	if (!msg->video.text) {
		return 0;
	}
	return file_exists((const char*)msg->video.text);
}

void PlayerMessage_disableSoundForNextMessage()
{
	playSound = 0;
}

void PlayerMessage_post(int usePopup, int messageType, int param1, short param2)
{
	int id = getNewMessageId();
	if (id < 0) {
		return;
	}
	Data_Message.totalMessages++;
	Data_Message.currentMessageId = id;

	struct Data_PlayerMessage *msg = &Data_Message.messages[id];
	msg->messageType = messageType;
	msg->readFlag = 0;
	msg->year = game_time_year();
	msg->month = game_time_month();
	msg->param1 = param1;
	msg->param2 = param2;
	msg->sequence = Data_Message.nextMessageSequence++;
	int textId = PlayerMessage_getMessageTextId(messageType);
	lang_message_type langMessageType = lang_get_message(textId)->message_type;
	if (langMessageType == MESSAGE_TYPE_DISASTER || langMessageType == MESSAGE_TYPE_INVASION) {
		Data_Message.hotspotCount = 1;
		UI_Window_requestRefresh();
	}
	if (usePopup && UI_Window_getId() == Window_City) {
		consecutiveMessageDelay = 5;
		Data_Message.currentProblemAreaMessageId = Data_Message.currentMessageId;
		msg->readFlag = 1;
		UI_Tooltip_resetTimer();
		if (!hasVideo(textId)) {
			if (lang_get_message(textId)->urgent == 1) {
				Sound_Effects_playChannel(SoundChannel_FanfareUrgent);
			} else {
				Sound_Effects_playChannel(SoundChannel_Fanfare);
			}
		}
		UI_MessageDialog_setPlayerMessage(
			msg->year, msg->month, msg->param1, msg->param2,
			PlayerMessage_getAdvisorForMessageType(msg->messageType), usePopup);
		UI_MessageDialog_show(textId, 0);
	} else if (usePopup) {
		// add to queue to be processed when player returns to city
		for (int i = 0; i < 20; i++) {
			if (!Data_Message.popupMessageQueue[i]) {
				Data_Message.popupMessageQueue[i] = msg->sequence;
				break;
			}
		}
	} else if (playSound) {
		if (lang_get_message(textId)->urgent == 1) {
			Sound_Effects_playChannel(SoundChannel_FanfareUrgent);
		} else {
			Sound_Effects_playChannel(SoundChannel_Fanfare);
		}
	}
	playSound = 1;
}

void PlayerMessage_postWithPopupDelay(int type, int messageType, int param1, short param2)
{
	int usePopup = 0;
	if (Data_Message.messageDelay[type] <= 0) {
		usePopup = 1;
		Data_Message.messageDelay[type] = 12;
	}
	PlayerMessage_post(usePopup, messageType, param1, param2);
	Data_Message.messageCategoryCount[type]++;
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
	struct Data_PlayerMessage *msg = &Data_Message.messages[msgId];
	msg->readFlag = 1;
	Data_Message.currentProblemAreaMessageId = msgId;
	int textId = PlayerMessage_getMessageTextId(msg->messageType);
	UI_Tooltip_resetTimer();
	if (!hasVideo(textId)) {
		if (lang_get_message(textId)->urgent == 1) {
			Sound_Effects_playChannel(SoundChannel_FanfareUrgent);
		} else {
			Sound_Effects_playChannel(SoundChannel_Fanfare);
		}
	}
	UI_MessageDialog_setPlayerMessage(
		msg->year, msg->month, msg->param1, msg->param2,
		PlayerMessage_getAdvisorForMessageType(msg->messageType), 1);
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
		case Message_22_LocalUprising:
		case Message_23_BarbarianAttack:
		case Message_24_CaesarArmyAttack:
		case Message_25_DistantBattle:
		case Message_26_EnemiesClosing:
		case Message_27_EnemiesAtTheDoor:
			return MessageAdvisor_Military;

		case Message_28_CaesarRequestsGoods:
		case Message_29_CaesarRequestsMoney:
		case Message_30_CaesarRequestsArmy:
		case Message_31_RequestReminder:
		case Message_32_RequestReceived:
		case Message_33_RequestRefused:
		case Message_34_RequestRefusedOverdue:
		case Message_35_RequestReceivedLate:
			return MessageAdvisor_Imperial;

		case Message_36_Unemployment:
		case Message_37_WorkersNeeded:
			return MessageAdvisor_Labor;

		case Message_49_NotEnoughFood:
		case Message_50_FoodNotDelivered:
			return MessageAdvisor_Population;

		case Message_102_HealthIllness:
		case Message_103_HealthDisease:
		case Message_104_HealthPestilence:
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
	city_message_init_scenario();
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
	Data_Message.hotspotCount = 0;
	Data_Message.hotspotIndex = 0;
	Data_Message.hotspotLastClick = time_get_millis();
}

void PlayerMessage_goToProblem()
{
	time_millis now = time_get_millis();
	if (now - Data_Message.hotspotLastClick > 3000) {
		Data_Message.hotspotIndex = 0;
	}
	Data_Message.hotspotLastClick = now;

	PlayerMessage_sortMessages();
	Data_Message.hotspotCount = 0;
	for (int i = 0; i < 999; i++) {
		struct Data_PlayerMessage *msg = &Data_Message.messages[i];
		if (msg->messageType && msg->year >= game_time_year() - 1) {
            const lang_message *lang_msg = lang_get_message(PlayerMessage_getMessageTextId(msg->messageType));
			lang_message_type langMessageType = lang_msg->message_type;
			if (langMessageType == MESSAGE_TYPE_DISASTER || langMessageType == MESSAGE_TYPE_INVASION) {
				if (langMessageType != MESSAGE_TYPE_INVASION || Formation_getInvasionGridOffset(msg->param1) > 0) {
					Data_Message.hotspotCount++;
				}
			}
		}
	}
	if (Data_Message.hotspotCount <= 0) {
		Data_Message.hotspotIndex = 0;
		return;
	}
	if (Data_Message.hotspotIndex >= Data_Message.hotspotCount) {
		Data_Message.hotspotIndex = 0;
	}
	int index = 0;
	for (int i = 0; i < 999; i++) {
		struct Data_PlayerMessage *msg = &Data_Message.messages[i];
		if (msg->messageType && msg->year >= game_time_year() - 1) {
			int textId = PlayerMessage_getMessageTextId(msg->messageType);
			lang_message_type langMessageType = lang_get_message(textId)->message_type;
			if (langMessageType == MESSAGE_TYPE_DISASTER || langMessageType == MESSAGE_TYPE_INVASION) {
				if (langMessageType != MESSAGE_TYPE_INVASION || Formation_getInvasionGridOffset(msg->param1) > 0) {
					index++;
					if (Data_Message.hotspotIndex < index) {
						Data_Message.hotspotIndex++;
						int gridOffset = msg->param2;
						if (langMessageType == MESSAGE_TYPE_INVASION) {
							gridOffset = Formation_getInvasionGridOffset(msg->param1);
						}
						if (gridOffset > 0) {
							CityView_goToGridOffset(gridOffset);
							UI_Window_goTo(Window_City);
						}
						return;
					}
				}
			}
		}
	}
	UI_Window_requestRefresh();
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

