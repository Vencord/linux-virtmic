#include <format>
#include <iostream>

#include <httplib.h>
#include <glaze/glaze.hpp>
#include <vencord/patchbay.hpp>

template <>
struct glz::meta<vencord::target_mode>
{
    using enum vencord::target_mode;
    static constexpr auto value = enumerate("include", include, "exclude", exclude);
};

template <>
struct glz::meta<vencord::target>
{
    using T                     = vencord::target;
    static constexpr auto value = object("key", &T::key, "value", &T::value, "mode", &T::mode);
};

int main(int argc, char **args)
{
    using vencord::patchbay;

    auto port = 7591;

    if (argc > 1)
    {
        try
        {
            port = std::stoi(args[1]);
        }
        catch (...)
        {
            std::cerr << std::format("Bad arguments, usage: {} [port]", args[0]) << std::endl;
            return 1;
        }
    }

    std::cout << std::format("Running on port {}\n", port);

    httplib::Server server;

    server.Get("/list",
               [](const auto &, auto &response)
               {
                   auto data = glz::write_json(patchbay::get().list());
                   response.set_content(data, "application/json");
               });

    server.Post("/link",
                [](const auto &req, auto &response)
                {
                    vencord::target parsed;
                    auto error = glz::read<glz::opts{.error_on_missing_keys = true}>(parsed, req.body);

                    if (error)
                    {
                        response.status = 418;
                        return;
                    }

                    patchbay::get().link(parsed);

                    response.status = 200;
                });

    server.Get("/unlink",
               [](const auto &, auto &response)
               {
                   patchbay::get().unlink();
                   response.status = 200;
               });

    server.listen("0.0.0.0", port);

    return 0;
}
