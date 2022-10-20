apps += rahat::App
app_headers += apps/rahat/app.hpp

app_srahat_src = $(addprefix apps/rahat/,\
  app.cpp \
  cs_list_controller.cpp \
  create_cs_controller.cpp \
)

apps_src += $(app_srahat_src)

app_images += apps/rahat/rahat_icon.png

i18n_files += $(call i18n_without_universal_for,rahat/base)

$(eval $(call depends_on_image,apps/rahat/app.cpp,apps/rahat/rahat_icon.png))
