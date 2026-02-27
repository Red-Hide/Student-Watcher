#define SENDER_CODE true

enum class AnimationType
{
    LOADING,
    ANIM_SUCCESS,
    ERROR,
    HEARTBEAT
};

void senderSetup();
void senderLoop();

void playAnimation(AnimationType);

void playSuccess(void);

void playError(void);

