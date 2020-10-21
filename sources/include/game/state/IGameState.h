#ifndef THEPROJECT2_IGAMESTATE_H
#define THEPROJECT2_IGAMESTATE_H

namespace game::state {
    class IGameState {
    public:
        virtual ~IGameState(){

        }

        virtual bool Initialize() = 0;
        virtual bool Finalize() = 0;
        virtual core::String GetName() = 0;
        virtual bool Run() = 0;
    };
}
#endif //THEPROJECT2_IGAMESTATE_H
