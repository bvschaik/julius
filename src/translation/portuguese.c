#include "translation/common.h"
#include "translation/translation.h"

static translation_string all_strings[] = {
   {TR_NO_PATCH_TITLE, "Atualização 1.0.1.0 não instalada"},
    {TR_NO_PATCH_MESSAGE,
        "Sua versão de Caesar III não tem a atualização 1.0.1.0 instalada. "
        "Você pode baixar a atualização em:\n"
        URL_PATCHES "\n"
        "Continue por seu próprio risco."},
    {TR_MISSING_FONTS_TITLE, "Fontes faltando"},
    {TR_MISSING_FONTS_MESSAGE,
        "Sua instalação de Caesar III precisa de outros arquivos de fontes. "
        "Você pode baixá-las para seu idioma em:\n"
        URL_PATCHES},
    {TR_NO_EDITOR_TITLE, "Editor não instalado"},
    {TR_NO_EDITOR_MESSAGE,
        "Sua versão de Caesar III não possui os arquivos do Editor de Mapas. "
        "Você pode baixá-lo em:\n"
        URL_EDITOR},
    {TR_INVALID_LANGUAGE_TITLE, "Pasta de idioma inválida"},
    {TR_INVALID_LANGUAGE_MESSAGE,
        "A pasta selecionada não contém um pacote de idioma válido. "
        "Verifique o arquivo de registro para ver os erros"},
    {TR_BUILD_ALL_TEMPLES, "Tudo"},
    {TR_BUTTON_OK, "OK"},
    {TR_BUTTON_CANCEL, "Cancelar"},
    {TR_BUTTON_RESET_DEFAULTS, "Redefinir padrões"},
    {TR_BUTTON_CONFIGURE_HOTKEYS, "Configurar atalhos"},
    {TR_CONFIG_TITLE, "Opções de Configurações do Julius"},
    {TR_CONFIG_LANGUAGE_LABEL, "Idioma:"},
    {TR_CONFIG_LANGUAGE_DEFAULT, "(padrão)"},
    {TR_CONFIG_DISPLAY_SCALE, "Escala de exibição:"},
    {TR_CONFIG_CURSOR_SCALE, "Escala do cursor:"},
    {TR_CONFIG_HEADER_UI_CHANGES, "Mudanças de interface"},
    {TR_CONFIG_HEADER_GAMEPLAY_CHANGES, "Mudanças de jogabilidade"},
    {TR_CONFIG_SHOW_INTRO_VIDEO, "Tocar vídeos de abertura"},
    {TR_CONFIG_SIDEBAR_INFO, "Informação extra no painel de controle"},
    {TR_CONFIG_SMOOTH_SCROLLING, "Ativar rolagem suave"},
    {TR_CONFIG_DISABLE_MOUSE_EDGE_SCROLLING, "Desativar rolagem do mapa no limite da tela"},
    {TR_CONFIG_DISABLE_RIGHT_CLICK_MAP_DRAG, "Desativar rolagem do mapa com botão direito do mouse"},
    {TR_CONFIG_INVERSE_RIGHT_CLICK_MAP_DRAG, "Arrastamento invertido do mapa com o botão direito do mouse"},
    {TR_CONFIG_VISUAL_FEEDBACK_ON_DELETE, "Ativar indicador visual ao limpar o terreno"},
    {TR_CONFIG_ALLOW_CYCLING_TEMPLES, "Permitir construir cada templo em sequência"},
    {TR_CONFIG_SHOW_WATER_STRUCTURE_RANGE, "Mostrar alcance ao construir reservatórios, fontes e poços"},
    {TR_CONFIG_SHOW_CONSTRUCTION_SIZE, "Mostrar o tamanho da construção ao arrastar"},
    {TR_CONFIG_HIGHLIGHT_LEGIONS, "Realçar legiões focadas pelo mouse"},
    {TR_CONFIG_SHOW_MILITARY_SIDEBAR, "Mostrar barra lateral militar"},
    {TR_CONFIG_FIX_IMMIGRATION_BUG, "Consertar falha durante a imigração na dificuldade máxima"},
    {TR_CONFIG_FIX_100_YEAR_GHOSTS, "Consertar falha dos 'fantasmas' de 100 anos"},
    {TR_HOTKEY_TITLE, "Configurações de teclas de atalho do Julius"},
    {TR_HOTKEY_LABEL, "Tecla de atalho"},
    {TR_HOTKEY_ALTERNATIVE_LABEL, "Alternativa"},
    {TR_HOTKEY_HEADER_ARROWS, "Teclas de seta"},
    {TR_HOTKEY_HEADER_GLOBAL, "Atalhos gerais"},
    {TR_HOTKEY_HEADER_CITY, "Atalhos da cidade"},
    {TR_HOTKEY_HEADER_ADVISORS, "Conselheiros"},
    {TR_HOTKEY_HEADER_OVERLAYS, "Mapa específicos"},
    {TR_HOTKEY_HEADER_BOOKMARKS, "Pontos de referência"},
    {TR_HOTKEY_HEADER_EDITOR, "Editor"},
    {TR_HOTKEY_HEADER_BUILD, "Atalhos de construção"},
    {TR_HOTKEY_ARROW_UP, "Cima"},
    {TR_HOTKEY_ARROW_DOWN, "Baixo"},
    {TR_HOTKEY_ARROW_LEFT, "Esquerda"},
    {TR_HOTKEY_ARROW_RIGHT, "Direita"},
    {TR_HOTKEY_TOGGLE_FULLSCREEN, "Tela Cheia"},
    {TR_HOTKEY_CENTER_WINDOW, "Centralizar"},
    {TR_HOTKEY_RESIZE_TO_640, "Redimensionar para 640x480"},
    {TR_HOTKEY_RESIZE_TO_800, "Redimensionar para 800x600"},
    {TR_HOTKEY_RESIZE_TO_1024, "Redimensionar para 1024x768"},
    {TR_HOTKEY_SAVE_SCREENSHOT, "Capturar tela"},
    {TR_HOTKEY_SAVE_CITY_SCREENSHOT, "Capturar imagem de cidade"},
    {TR_HOTKEY_BUILD_CLONE, "Clonar construção sob o cursor"},
    {TR_HOTKEY_LOAD_FILE, "Carregar jogo"},
    {TR_HOTKEY_SAVE_FILE, "Salvar jogo"},
    {TR_HOTKEY_INCREASE_GAME_SPEED, "Aumentar velocidade do jogo"},
    {TR_HOTKEY_DECREASE_GAME_SPEED, "Reduzir velocidade do jogo"},
    {TR_HOTKEY_TOGGLE_PAUSE, "Dar pausa"},
    {TR_HOTKEY_CYCLE_LEGION, "Circular por legiões"},
    {TR_HOTKEY_ROTATE_MAP_LEFT, "Girar no sentido anti-horário"},
    {TR_HOTKEY_ROTATE_MAP_RIGHT, "Girar no sentido horário"},
    {TR_HOTKEY_SHOW_ADVISOR_LABOR, "Trabalho"},
    {TR_HOTKEY_SHOW_ADVISOR_MILITARY, "Militar"},
    {TR_HOTKEY_SHOW_ADVISOR_IMPERIAL, "Imperial"},
    {TR_HOTKEY_SHOW_ADVISOR_RATINGS, "Índices"},
    {TR_HOTKEY_SHOW_ADVISOR_TRADE, "Comércio Exterior"},
    {TR_HOTKEY_SHOW_ADVISOR_POPULATION, "População"},
    {TR_HOTKEY_SHOW_ADVISOR_HEALTH, "Saúde"},
    {TR_HOTKEY_SHOW_ADVISOR_EDUCATION, "Educação"},
    {TR_HOTKEY_SHOW_ADVISOR_ENTERTAINMENT, "Entretenimento"},
    {TR_HOTKEY_SHOW_ADVISOR_RELIGION, "Religião"},
    {TR_HOTKEY_SHOW_ADVISOR_FINANCIAL, "Finanças"},
    {TR_HOTKEY_SHOW_ADVISOR_CHIEF, "Conselheiro Chefe"},
    {TR_HOTKEY_TOGGLE_OVERLAY, "Alternar Mapa Atual/Padrão"},
    {TR_HOTKEY_SHOW_OVERLAY_WATER, "Mapa de Água"},
    {TR_HOTKEY_SHOW_OVERLAY_FIRE, "Mapa de Fogo"},
    {TR_HOTKEY_SHOW_OVERLAY_DAMAGE, "Mapa de Avarias"},
    {TR_HOTKEY_SHOW_OVERLAY_CRIME, "Mapa de Criminalidade"},
    {TR_HOTKEY_SHOW_OVERLAY_PROBLEMS, "Mapa de Problemas"},
    {TR_HOTKEY_GO_TO_BOOKMARK_1, "Ir para marcador 1"},
    {TR_HOTKEY_GO_TO_BOOKMARK_2, "Ir para marcador 2"},
    {TR_HOTKEY_GO_TO_BOOKMARK_3, "Ir para marcador 3"},
    {TR_HOTKEY_GO_TO_BOOKMARK_4, "Ir para marcador 4"},
    {TR_HOTKEY_SET_BOOKMARK_1, "Definir marcador 1"},
    {TR_HOTKEY_SET_BOOKMARK_2, "Definir marcador 2"},
    {TR_HOTKEY_SET_BOOKMARK_3, "Definir marcador 3"},
    {TR_HOTKEY_SET_BOOKMARK_4, "Definir marcador 4"},
    {TR_HOTKEY_EDITOR_TOGGLE_BATTLE_INFO, "Mostrar informações de batalha"},
    {TR_HOTKEY_EDIT_TITLE, "Nova tecla de atalho"},
    {TR_HOTKEY_DUPLICATE_TITLE, "Atalho já utilizado"},
    {TR_HOTKEY_DUPLICATE_MESSAGE, "Esta combinação de teclas já está designada para a seguinte ação:"},
    {TR_WARNING_SCREENSHOT_SAVED, "Captura de tela salva: "},
};

void translation_portuguese(const translation_string **strings, int *num_strings)
{
    *strings = all_strings;
    *num_strings = sizeof(all_strings) / sizeof(translation_string);
}
