#include <pu/ui/elm/elm_Button.hpp>

namespace pu::ui::elm
{
    Button::Button(s32 X, s32 Y, s32 Width, s32 Height, const std::string& Content, Color TextColor, Color Color)
        : Element::Element(), x(X), y(Y), w(Width), h(Height), cnt(Content), clr(Color)
    {
        this->hover = false;
        this->hoverfact = 255;
        this->SetFontSize(25);
        this->clickcb = [](){};
    }

    Button::~Button()
    {
        render::DeleteTexture(this->ntex);
    }

    s32 Button::GetX()
    {
        return this->x;
    }

    void Button::SetX(s32 X)
    {
        this->x = X;
    }

    s32 Button::GetY()
    {
        return this->y;
    }

    void Button::SetY(s32 Y)
    {
        this->y = Y;
    }

    s32 Button::GetWidth()
    {
        return this->w;
    }

    void Button::SetWidth(s32 Width)
    {
        this->w = Width;
    }

    s32 Button::GetHeight()
    {
        return this->h;
    }

    void Button::SetHeight(s32 Height)
    {
        this->h = Height;
    }

    std::string Button::GetContent()
    {
        return this->cnt;
    }

    void Button::SetContent(const std::string& Content)
    {
        this->cnt = Content;
        render::DeleteTexture(this->ntex);
        this->ntex = render::RenderText(this->font, this->meme, Content, this->clr);
    }

    Color Button::GetColor()
    {
        return this->clr;
    }

    void Button::SetColor(Color Color)
    {
        this->clr = Color;
        render::DeleteTexture(this->ntex);
        this->ntex = render::RenderText(this->font, this->meme, this->cnt, Color);
    }

    void Button::SetFontSize(s32 FontSize)
    {
        this->font = render::LoadDefaultFont(FontSize);
        this->meme = render::LoadSharedFont(render::SharedFont::NintendoExtended, FontSize);
        render::DeleteTexture(this->ntex);
        this->ntex = render::RenderText(this->font, this->meme, this->cnt, this->clr);
    }

    void Button::SetOnClick(std::function<void()> ClickCallback)
    {
        this->clickcb = ClickCallback;
    }

    void Button::OnRender(render::Renderer::Ref &Drawer, s32 X, s32 Y)
    {
        s32 rdx = X;
        s32 rdy = Y;
        s32 clrr = this->clr.R;
        s32 clrg = this->clr.G;
        s32 clrb = this->clr.B;
        s32 nr = clrr - 70;
        if(nr < 0) nr = 0;
        s32 ng = clrg - 70;
        if(ng < 0) ng = 0;
        s32 nb = clrb - 70;
        if(nb < 0) nb = 0;
        Color nclr(nr, ng, nb, this->clr.A);
        if(this->hover)
        {
            Drawer->RenderRectangleFill(this->clr, rdx, rdy, this->w, this->h);
            if(this->hoverfact < 255)
            {
                Drawer->RenderRectangleFill({ nr, ng, nb, this->hoverfact }, rdx, rdy, this->w, this->h);
                this->hoverfact += 48;
            }
            else Drawer->RenderRectangleFill(nclr, rdx, rdy, this->w, this->h);
        }
        else
        {
            Drawer->RenderRectangleFill(this->clr, rdx, rdy, this->w, this->h);
            if(this->hoverfact > 0)
            {
                Drawer->RenderRectangleFill({ nr, ng, nb, this->hoverfact }, rdx, rdy, this->w, this->h);
                this->hoverfact -= 48;
            }
            else Drawer->RenderRectangleFill(this->clr, rdx, rdy, this->w, this->h);
        }
        auto [xw,xh] = render::GetTextureSize(this->ntex);
        s32 tx = ((this->w - xw) / 2) + rdx;
        s32 ty = ((this->h - xh) / 2) + rdy;
        Drawer->RenderTexture(this->ntex, tx, ty);
    }

    void Button::OnInput(u64 Down, u64 Up, u64 Held, Touch Pos)
    {
        if(this->hover)
        {
            if(Pos.IsEmpty())
            {
                (this->clickcb)();
                this->hover = false;
                this->hoverfact = 255;
            }
        }
        else
        {
            if(!Pos.IsEmpty())
            {
                if((Pos.X >= x) && (Pos.X < (x + w)) && (Pos.Y >= y) && (Pos.Y < (y + h)))
                {
                    this->hover = true;
                    this->hoverfact = 0;
                }
            }
        }
    }
}