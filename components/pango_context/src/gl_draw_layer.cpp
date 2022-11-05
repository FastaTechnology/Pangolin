#include <pangolin/gui/draw_layer.h>
#include <pangolin/context/context.h>
#include <pangolin/context/factory.h>
#include <pangolin/handler/handler.h>
#include <pangolin/gl/glsl_program.h>
#include <pangolin/gl/uniform.h>
#include <pangolin/gl/gl.h>
#include <pangolin/gl/glvao.h>
#include <pangolin/utils/variant_overload.h>
#include <pangolin/maths/camera_look_at.h>
#include <pangolin/maths/projection.h>

#include "glutils.h"

#include <unordered_map>

namespace pangolin
{

struct DrawnPrimitivesProgram
{
    void draw(const DrawnPrimitives& drawn_image)
    {
        // auto bind_im = drawn_image.image->bind();
        auto bind_prog = prog->bind();
        auto bind_vao = vao.bind();

    }

private:
    const Shared<GlSlProgram> prog = GlSlProgram::Create({
        .sources = {{ .origin="/components/pango_opengl/shaders/main_primitives_points.glsl" }}
    });
    GlVertexArrayObject vao = {};
    const GlUniform<Eigen::Matrix4f> K_intrinsics = {"K_intrinsics"};
    const GlUniform<Eigen::Matrix4f> T_world_image = {"T_world_image"};
    const GlUniform<Eigen::Vector2f> image_size = {"image_size"};

};

struct DrawnImageProgram
{
    DrawnImageProgram()
    {
    }

    void draw(const DrawnImage& drawn_image)
    {
        // PANGO_GL(glEnable(GL_DEPTH_TEST));
        PANGO_GL(glActiveTexture(GL_TEXTURE0));
        auto bind_im = drawn_image.image->bind();
        auto bind_prog = prog->bind();
        auto bind_vao = vao.bind();

        // image_size = Eigen::Vector2f(
        //     1.0f, 1.0f
        //     // drawn_image.image->imageSize().width,
        //     // drawn_image.image->imageSize().height
        // );
        // K_intrinsics = Eigen::Matrix4f::Identity();
        // projectionClipFromCamera(
        //     drawn_image.image->imageSize(),
        //     1.0, {0.0,0.0}, {0.1, 100.0}
        // ).cast<float>();

        // println("K:\n{}\n", K_intrinsics.getValue());

        // T_world_image = worldLookatFromCamera<float>(
        //     {0.0, 0.0, -1.0},
        //     {0.0, 0.0, 0.0},
        //     {0.0, -1.0, 0.0}
        // ).matrix();
        // sophus::Se3d T;
        // T.tran

        PANGO_GL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    }
private:
    const Shared<GlSlProgram> prog = GlSlProgram::Create({
        .sources = {{ .origin="/components/pango_opengl/shaders/main_image.glsl" }}
    });
    GlVertexArrayObject vao = {};
    const GlUniform<int> texture_unit = {"image"};
    const GlUniform<Eigen::Matrix4f> K_intrinsics = {"K_intrinsics"};
    const GlUniform<Eigen::Matrix4f> T_world_image = {"T_world_image"};
    const GlUniform<Eigen::Vector2f> image_size = {"image_size"};
};

struct DrawLayerImpl : public DrawLayer {
    Eigen::Array3f debug_random_color;

    DrawLayerImpl(const DrawLayerImpl::Params& p)
        : name_(p.name),
        size_hint_(p.size_hint),
        near_far_(p.near_far),
        camera_(p.camera),
        world_from_camera_(p.world_from_camera),
        handler_(p.handler),
        cam_from_world_(Eigen::Matrix4d::Identity()),
        intrinsic_k_(Eigen::Matrix4d::Identity()),
        non_linear_(),
        objects_(p.objects)
    {
        debug_random_color = (Eigen::Array3f::Random() + 1.0f) / 2.0;
    }

    std::string name() const override
    {
        return name_;
    }

    void renderIntoRegion(const Context& c, const RenderParams& p) override {
        ScopedGlEnable en_scissor(GL_SCISSOR_TEST);
        c.setViewport(p.region);
        glClearColor(debug_random_color[0], debug_random_color[2], debug_random_color[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for(auto& obj : objects_) {
            if(DrawnImage* im = dynamic_cast<DrawnImage*>(obj.ptr())) {
                render_image.draw(*im);
            }
        }
    }

    bool handleEvent(const Context& context, const Event& event) override {
        if(handler_) {
            handler_->handleEvent( *this, *world_from_camera_, *camera_, near_far_, context, event );
            return true;
        }
        return false;
    }

    Size sizeHint() const override {
        return size_hint_;
    }

    void setHandler(const std::shared_ptr<Handler>& handler) override {
        handler_ = handler;
    }

    MinMax<Eigen::Vector3d> getSceneBoundsInWorld() const override {
        return bounds_;
    }

    void add(const Shared<Drawable>& r) override {
        objects_.push_back(r);
    }

    void remove(const Shared<Drawable>& r) override {
        throw std::runtime_error("Not implemented yet...");
    }

    void clear() override {
        objects_.clear();
    }

    std::string name_;
    Size size_hint_;
    MinMax<double> near_far_;
    MinMax<Eigen::Vector3d> bounds_;

    // Used for handling and higher-level logic
    Shared<sophus::CameraModel> camera_;
    Shared<sophus::Se3F64> world_from_camera_;
    std::shared_ptr<Handler> handler_;

    // Actually used for rendering
    Eigen::Matrix4d cam_from_world_;
    Eigen::Matrix4d intrinsic_k_;
    NonLinearMethod non_linear_;

    std::vector<Shared<Drawable>> objects_;
    DrawnImageProgram render_image;
};

PANGO_CREATE(DrawLayer) {
    return Shared<DrawLayerImpl>::make(p);
}

}