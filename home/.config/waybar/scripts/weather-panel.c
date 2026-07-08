#define _XOPEN_SOURCE 700

#include <gtk/gtk.h>
#include <json-glib/json-glib.h>
#include <time.h>

typedef struct {
    double lo;
    double hi;
    double week_lo;
    double week_hi;
} RangeData;

typedef struct {
    char time[16];
    int code;
    double temp;
    int rain;
} Hour;

typedef struct {
    char day[16];
    int code;
    double lo;
    double hi;
    int rain;
} Day;

typedef struct {
    char location[160];
    char condition[48];
    char today_note[160];
    char updated[64];
    int code;
    double current;
    double feels;
    double high;
    double low;
    int rain;
    int humidity;
    double wind;
    double gust;
    double week_lo;
    double week_hi;
    Hour hours[6];
    int n_hours;
    Day days[7];
    int n_days;
} WeatherData;

static WeatherData weather;

static GtkWidget * label_new(const char * text, const char * css_class) {
    GtkWidget * label = gtk_label_new(text);
    gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
    if (css_class && css_class[0]) {
        gtk_widget_add_css_class(label, css_class);
    }
    return label;
}

static void add_class(GtkWidget * widget, const char * css_class) {
    gtk_widget_add_css_class(widget, css_class);
}

static const char * weather_text(int code) {
    if (code == 0) return "Clear";
    if (code == 1) return "Mostly clear";
    if (code == 2) return "Partly cloudy";
    if (code == 3) return "Cloudy";
    if (code == 45 || code == 48) return "Fog";
    if (code >= 51 && code <= 57) return "Drizzle";
    if (code >= 61 && code <= 67) return "Rain";
    if (code >= 71 && code <= 77) return "Snow";
    if (code >= 80 && code <= 82) return "Showers";
    if (code >= 85 && code <= 86) return "Snow showers";
    if (code >= 95 && code <= 99) return "Thunderstorm";
    return "Weather";
}

static const char * weather_icon(int code) {
    if (code == 0) return "󰖙";
    if (code == 1 || code == 2) return "󰖕";
    if (code == 3) return "󰖐";
    if (code == 45 || code == 48) return "󰖑";
    if ((code >= 51 && code <= 67) || (code >= 80 && code <= 82)) return "󰖗";
    if ((code >= 71 && code <= 77) || (code >= 85 && code <= 86)) return "󰖘";
    if (code >= 95 && code <= 99) return "󰖓";
    return "󰖐";
}

static int round_int(double value) {
    return (int) (value + (value >= 0 ? 0.5 : -0.5));
}

static void temp_text(char * out, size_t size, double value) {
    g_snprintf(out, size, "%d°C", round_int(value));
}

static void pct_text(char * out, size_t size, int value) {
    if (value < 10) {
        g_strlcpy(out, "--", size);
    } else {
        g_snprintf(out, size, "%d%%", value);
    }
}

static JsonObject * object_member(JsonObject * obj, const char * name) {
    if (!obj || !json_object_has_member(obj, name)) return NULL;
    JsonNode * node = json_object_get_member(obj, name);
    if (!node || !JSON_NODE_HOLDS_OBJECT(node)) return NULL;
    return json_node_get_object(node);
}

static JsonArray * array_member(JsonObject * obj, const char * name) {
    if (!obj || !json_object_has_member(obj, name)) return NULL;
    JsonNode * node = json_object_get_member(obj, name);
    if (!node || !JSON_NODE_HOLDS_ARRAY(node)) return NULL;
    return json_node_get_array(node);
}

static double number_member(JsonObject * obj, const char * name, double fallback) {
    if (!obj || !json_object_has_member(obj, name)) return fallback;
    JsonNode * node = json_object_get_member(obj, name);
    if (!node || !JSON_NODE_HOLDS_VALUE(node)) return fallback;
    return json_node_get_double(node);
}

static const char * string_member(JsonObject * obj, const char * name, const char * fallback) {
    if (!obj || !json_object_has_member(obj, name)) return fallback;
    JsonNode * node = json_object_get_member(obj, name);
    if (!node || !JSON_NODE_HOLDS_VALUE(node)) return fallback;
    const char * value = json_node_get_string(node);
    return value ? value : fallback;
}

static double array_number(JsonArray * arr, guint index, double fallback) {
    if (!arr || index >= json_array_get_length(arr)) return fallback;
    JsonNode * node = json_array_get_element(arr, index);
    if (!node || !JSON_NODE_HOLDS_VALUE(node)) return fallback;
    return json_node_get_double(node);
}

static const char * array_string(JsonArray * arr, guint index, const char * fallback) {
    if (!arr || index >= json_array_get_length(arr)) return fallback;
    JsonNode * node = json_array_get_element(arr, index);
    if (!node || !JSON_NODE_HOLDS_VALUE(node)) return fallback;
    const char * value = json_node_get_string(node);
    return value ? value : fallback;
}

static void clock_label(const char * iso, char * out, size_t size) {
    if (!iso || strlen(iso) < 13) {
        g_strlcpy(out, "--", size);
        return;
    }
    int hour = atoi(iso + 11);
    int display = hour % 12;
    if (display == 0) display = 12;
    g_snprintf(out, size, "%d %s", display, hour < 12 ? "AM" : "PM");
}

static void day_label(const char * date, const char * current_date, char * out, size_t size) {
    if (date && current_date && g_strcmp0(date, current_date) == 0) {
        g_strlcpy(out, "Today", size);
        return;
    }

    struct tm tm = {0};
    if (date && strptime(date, "%Y-%m-%d", &tm)) {
        mktime(&tm);
        strftime(out, size, "%a", &tm);
        return;
    }
    g_strlcpy(out, "--", size);
}

static gboolean load_weather(const char * path) {
    g_autoptr(JsonParser) parser = json_parser_new();
    g_autoptr(GError) error = NULL;

    if (!json_parser_load_from_file(parser, path, &error)) {
        g_printerr("weather-panel: %s\n", error ? error->message : "cannot load weather json");
        return FALSE;
    }

    JsonNode * root_node = json_parser_get_root(parser);
    if (!root_node || !JSON_NODE_HOLDS_OBJECT(root_node)) return FALSE;

    JsonObject * root = json_node_get_object(root_node);
    JsonObject * loc = object_member(root, "location");
    JsonObject * forecast = object_member(root, "forecast");
    JsonObject * current = object_member(forecast, "current");
    JsonObject * hourly = object_member(forecast, "hourly");
    JsonObject * daily = object_member(forecast, "daily");

    const char * name = string_member(loc, "name", "Weather");
    const char * admin = string_member(loc, "admin1", "");
    if (admin[0]) {
        g_snprintf(weather.location, sizeof weather.location, "%s, %s", name, admin);
    } else {
        g_strlcpy(weather.location, name, sizeof weather.location);
    }

    const char * current_time = string_member(current, "time", "");
    char current_date[11] = "";
    if (strlen(current_time) >= 10) {
        memcpy(current_date, current_time, 10);
        current_date[10] = '\0';
    }

    weather.current = number_member(current, "temperature_2m", 0);
    weather.feels = number_member(current, "apparent_temperature", weather.current);
    weather.code = round_int(number_member(current, "weather_code", 3));
    weather.humidity = round_int(number_member(current, "relative_humidity_2m", 0));
    weather.wind = number_member(current, "wind_speed_10m", 0);
    weather.gust = number_member(current, "wind_gusts_10m", 0);
    g_strlcpy(weather.condition, weather_text(weather.code), sizeof weather.condition);

    JsonArray * daily_time = array_member(daily, "time");
    JsonArray * daily_code = array_member(daily, "weather_code");
    JsonArray * daily_hi = array_member(daily, "temperature_2m_max");
    JsonArray * daily_lo = array_member(daily, "temperature_2m_min");
    JsonArray * daily_rain = array_member(daily, "precipitation_probability_max");

    weather.high = array_number(daily_hi, 0, weather.current);
    weather.low = array_number(daily_lo, 0, weather.current);
    weather.rain = round_int(array_number(daily_rain, 0, 0));
    weather.week_lo = 200;
    weather.week_hi = -200;

    guint n_days = daily_time ? MIN(json_array_get_length(daily_time), 7) : 0;
    weather.n_days = (int) n_days;
    for (guint i = 0; i < n_days; i++) {
        Day * day = &weather.days[i];
        day_label(array_string(daily_time, i, ""), current_date, day->day, sizeof day->day);
        day->code = round_int(array_number(daily_code, i, 3));
        day->lo = array_number(daily_lo, i, weather.current);
        day->hi = array_number(daily_hi, i, weather.current);
        day->rain = round_int(array_number(daily_rain, i, 0));
        if (day->lo < weather.week_lo) weather.week_lo = day->lo;
        if (day->hi > weather.week_hi) weather.week_hi = day->hi;
    }
    if (weather.week_lo > weather.week_hi) {
        weather.week_lo = weather.low;
        weather.week_hi = weather.high;
    }

    JsonArray * hourly_time = array_member(hourly, "time");
    JsonArray * hourly_code = array_member(hourly, "weather_code");
    JsonArray * hourly_temp = array_member(hourly, "temperature_2m");
    JsonArray * hourly_rain = array_member(hourly, "precipitation_probability");

    guint hour0 = 0;
    guint n_hourly = hourly_time ? json_array_get_length(hourly_time) : 0;
    for (guint i = 0; i < n_hourly; i++) {
        const char * candidate = array_string(hourly_time, i, "");
        if (g_strcmp0(candidate, current_time) >= 0) {
            hour0 = i;
            break;
        }
    }

    const guint offsets[] = {0, 2, 4, 6, 9, 11};
    weather.n_hours = 0;
    for (guint i = 0; i < G_N_ELEMENTS(offsets); i++) {
        guint index = hour0 + offsets[i];
        if (index >= n_hourly) continue;
        Hour * hour = &weather.hours[weather.n_hours++];
        clock_label(array_string(hourly_time, index, ""), hour->time, sizeof hour->time);
        hour->code = round_int(array_number(hourly_code, index, 3));
        hour->temp = array_number(hourly_temp, index, weather.current);
        hour->rain = round_int(array_number(hourly_rain, index, 0));
    }

    int next_rain = -1;
    char rain_time[16] = "";
    for (guint i = hour0; i < MIN(hour0 + 12, n_hourly); i++) {
        int rain = round_int(array_number(hourly_rain, i, 0));
        if (rain >= 45) {
            next_rain = rain;
            clock_label(array_string(hourly_time, i, ""), rain_time, sizeof rain_time);
            break;
        }
    }

    if (next_rain >= 0) {
        g_snprintf(weather.today_note, sizeof weather.today_note,
                   "Rain likely around %s (%d%%). High %d°C, low %d°C.",
                   rain_time, next_rain, round_int(weather.high), round_int(weather.low));
    } else if (weather.rain >= 35) {
        g_snprintf(weather.today_note, sizeof weather.today_note,
                   "Rain chance peaks at %d%% today. High %d°C, low %d°C.",
                   weather.rain, round_int(weather.high), round_int(weather.low));
    } else {
        g_snprintf(weather.today_note, sizeof weather.today_note,
                   "No meaningful rain expected today. High %d°C, low %d°C.",
                   round_int(weather.high), round_int(weather.low));
    }

    if (current_time[0]) {
        char clock[16];
        clock_label(current_time, clock, sizeof clock);
        g_snprintf(weather.updated, sizeof weather.updated, "Updated %s", clock);
    } else {
        g_strlcpy(weather.updated, "Updated recently", sizeof weather.updated);
    }

    return TRUE;
}

static GtkWidget * metric_chip(const char * label, const char * value, const char * extra_class) {
    GtkWidget * chip = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    add_class(chip, "chip");
    if (extra_class) add_class(chip, extra_class);
    gtk_widget_set_hexpand(chip, TRUE);

    GtkWidget * label_widget = label_new(label, "muted");
    gtk_widget_add_css_class(label_widget, "tiny");
    GtkWidget * value_widget = label_new(value, "chip-value");
    gtk_widget_set_halign(value_widget, GTK_ALIGN_CENTER);
    gtk_label_set_xalign(GTK_LABEL(label_widget), 0.5f);
    gtk_label_set_xalign(GTK_LABEL(value_widget), 0.5f);

    gtk_box_append(GTK_BOX(chip), label_widget);
    gtk_box_append(GTK_BOX(chip), value_widget);
    return chip;
}

static void draw_range(GtkDrawingArea * area, cairo_t * cr, int width, int height, gpointer user_data) {
    (void) area;
    RangeData * range = user_data;
    double span = range->week_hi - range->week_lo;
    if (span <= 0.1) span = 1.0;

    double start = ((range->lo - range->week_lo) / span) * width;
    double end = ((range->hi - range->week_lo) / span) * width;
    double y = height / 2.0;

    cairo_set_line_width(cr, 5.0);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_source_rgb(cr, 0.145, 0.220, 0.192);
    cairo_move_to(cr, 3, y);
    cairo_line_to(cr, width - 3, y);
    cairo_stroke(cr);

    cairo_set_source_rgb(cr, 0.561, 0.749, 0.451);
    cairo_move_to(cr, MAX(3, start), y);
    cairo_line_to(cr, MIN(width - 3, end), y);
    cairo_stroke(cr);
}

static GtkWidget * weather_panel(void) {
    GtkWidget * root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    add_class(root, "root");

    GtkWidget * header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    add_class(header, "header");
    gtk_box_append(GTK_BOX(root), header);

    GtkWidget * now = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_hexpand(now, TRUE);
    gtk_box_append(GTK_BOX(header), now);

    GtkWidget * icon = label_new(weather_icon(weather.code), "hero-icon");
    gtk_widget_set_valign(icon, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(now), icon);

    GtkWidget * now_text = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_widget_set_valign(now_text, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(now), now_text);

    char temp[16], feels[24], high[16], low[16], rain[16], wind[32], gust[32], humidity[32];
    temp_text(temp, sizeof temp, weather.current);
    temp_text(high, sizeof high, weather.high);
    temp_text(low, sizeof low, weather.low);
    pct_text(rain, sizeof rain, weather.rain);
    g_snprintf(feels, sizeof feels, "Feels %d°C", round_int(weather.feels));
    g_snprintf(wind, sizeof wind, "%.0f mph", weather.wind);
    g_snprintf(gust, sizeof gust, "%.0f mph", weather.gust);
    g_snprintf(humidity, sizeof humidity, "%d%%", weather.humidity);

    GtkWidget * title_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_append(GTK_BOX(now_text), title_row);
    gtk_box_append(GTK_BOX(title_row), label_new(temp, "hero-temp"));
    gtk_box_append(GTK_BOX(title_row), label_new(weather.condition, "condition"));
    gtk_box_append(GTK_BOX(now_text), label_new(weather.location, "muted"));
    gtk_box_append(GTK_BOX(now_text), label_new(feels, "muted"));

    GtkWidget * close = gtk_button_new_with_label("×");
    add_class(close, "close");
    gtk_widget_set_halign(close, GTK_ALIGN_END);
    gtk_widget_set_valign(close, GTK_ALIGN_START);
    gtk_widget_set_size_request(close, 32, 32);
    gtk_box_append(GTK_BOX(header), close);

    GtkWidget * metrics = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_append(GTK_BOX(root), metrics);
    gtk_box_append(GTK_BOX(metrics), metric_chip("High", high, NULL));
    gtk_box_append(GTK_BOX(metrics), metric_chip("Low", low, NULL));
    gtk_box_append(GTK_BOX(metrics), metric_chip("Rain", rain, weather.rain >= 35 ? "rainy" : NULL));
    gtk_box_append(GTK_BOX(metrics), metric_chip("Wind", wind, NULL));
    gtk_box_append(GTK_BOX(metrics), metric_chip("Gust", gust, NULL));
    gtk_box_append(GTK_BOX(metrics), metric_chip("Humidity", humidity, NULL));

    GtkWidget * today = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    add_class(today, "card");
    if (weather.rain >= 35) add_class(today, "notice");
    gtk_box_append(GTK_BOX(root), today);
    gtk_box_append(GTK_BOX(today), label_new("Today", "section"));
    GtkWidget * note = label_new(weather.today_note, "body");
    gtk_label_set_wrap(GTK_LABEL(note), TRUE);
    gtk_box_append(GTK_BOX(today), note);

    GtkWidget * hourly = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    add_class(hourly, "card");
    gtk_box_append(GTK_BOX(root), hourly);
    gtk_box_append(GTK_BOX(hourly), label_new("Next 12 Hours", "section"));

    GtkWidget * hour_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_append(GTK_BOX(hourly), hour_row);
    for (int i = 0; i < weather.n_hours; i++) {
        Hour * hour = &weather.hours[i];
        GtkWidget * col = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
        add_class(col, "hour");
        if (hour->rain >= 35) add_class(col, "wet-hour");
        gtk_widget_set_hexpand(col, TRUE);

        char hour_temp[16], hour_rain[16];
        temp_text(hour_temp, sizeof hour_temp, hour->temp);
        pct_text(hour_rain, sizeof hour_rain, hour->rain);

        GtkWidget * time = label_new(hour->time, "muted");
        GtkWidget * sky = label_new(weather_icon(hour->code), "hour-icon");
        GtkWidget * t = label_new(hour_temp, "hour-temp");
        GtkWidget * r = label_new(hour_rain, hour->rain >= 35 ? "rain-text" : "muted");
        gtk_label_set_xalign(GTK_LABEL(time), 0.5f);
        gtk_label_set_xalign(GTK_LABEL(sky), 0.5f);
        gtk_label_set_xalign(GTK_LABEL(t), 0.5f);
        gtk_label_set_xalign(GTK_LABEL(r), 0.5f);

        gtk_box_append(GTK_BOX(col), time);
        gtk_box_append(GTK_BOX(col), sky);
        gtk_box_append(GTK_BOX(col), t);
        gtk_box_append(GTK_BOX(col), r);
        gtk_box_append(GTK_BOX(hour_row), col);
    }

    GtkWidget * weekly = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    add_class(weekly, "card");
    gtk_box_append(GTK_BOX(root), weekly);
    gtk_box_append(GTK_BOX(weekly), label_new("7 Days", "section"));

    GtkWidget * grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 7);
    gtk_box_append(GTK_BOX(weekly), grid);

    gtk_grid_attach(GTK_GRID(grid), label_new("Day", "muted"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_new("Lo", "muted"), 2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_new("Range", "muted"), 3, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_new("Hi", "muted"), 4, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_new("Rain", "muted"), 5, 0, 1, 1);

    for (int i = 0; i < weather.n_days; i++) {
        Day * day = &weather.days[i];
        int row = i + 1;

        char day_low[16], day_high[16], day_rain[16];
        temp_text(day_low, sizeof day_low, day->lo);
        temp_text(day_high, sizeof day_high, day->hi);
        pct_text(day_rain, sizeof day_rain, day->rain);

        GtkWidget * day_name = label_new(day->day, i == 0 ? "today-label" : NULL);
        GtkWidget * day_icon = label_new(weather_icon(day->code), "day-icon");
        GtkWidget * low_label = label_new(day_low, "muted");
        GtkWidget * high_label = label_new(day_high, NULL);
        GtkWidget * rain_label = label_new(day_rain, day->rain >= 35 ? "rain-text" : "muted");

        GtkWidget * range = gtk_drawing_area_new();
        gtk_widget_set_size_request(range, 160, 14);
        gtk_widget_set_hexpand(range, TRUE);

        RangeData * range_data = g_new0(RangeData, 1);
        range_data->lo = day->lo;
        range_data->hi = day->hi;
        range_data->week_lo = weather.week_lo;
        range_data->week_hi = weather.week_hi;
        gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(range), draw_range, range_data, g_free);

        gtk_grid_attach(GTK_GRID(grid), day_name, 0, row, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), day_icon, 1, row, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), low_label, 2, row, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), range, 3, row, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), high_label, 4, row, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), rain_label, 5, row, 1, 1);
    }

    gtk_box_append(GTK_BOX(root), label_new(weather.updated, "updated"));
    return root;
}

static gboolean on_key(GtkEventControllerKey * controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data) {
    (void) controller;
    (void) keycode;
    (void) state;
    if (keyval == GDK_KEY_Escape) {
        gtk_window_destroy(GTK_WINDOW(user_data));
        return TRUE;
    }
    return FALSE;
}

static void activate(GtkApplication * app, gpointer user_data) {
    (void) user_data;

    static const char * css =
        "window { background: #0b1412; color: #dbe6dc; }"
        ".root { padding: 18px; background: #0b1412; border: 1px solid #5f8f58; border-radius: 14px; }"
        ".header { margin-bottom: 4px; }"
        ".hero-icon { font-size: 46px; color: #c6b575; }"
        ".hero-temp { font-size: 43px; font-weight: 800; color: #dbe6dc; }"
        ".condition { font-size: 18px; font-weight: 800; color: #c6b575; margin-top: 14px; }"
        ".muted { color: #93aaa0; }"
        ".tiny { font-size: 11px; }"
        ".body { font-size: 14px; }"
        ".card { background: #121d1b; border-radius: 12px; padding: 12px; border: 1px solid #253831; }"
        ".notice { background: #111f20; border-color: #3d5d65; }"
        ".section { font-size: 13px; font-weight: 800; color: #8fbf73; margin-bottom: 4px; }"
        ".chip { background: #182722; border-radius: 999px; padding: 8px 12px; border: 1px solid #253831; min-width: 66px; }"
        ".chip-value { font-weight: 800; color: #dbe6dc; }"
        ".rainy .chip-value, .rain-text { color: #9db6c1; font-weight: 800; }"
        ".hour { background: #182722; border-radius: 10px; padding: 9px 6px; border: 1px solid transparent; }"
        ".wet-hour { background: #172a2d; border-color: #3d5d65; }"
        ".hour-icon { font-size: 20px; color: #c6b575; }"
        ".hour-temp { font-size: 16px; font-weight: 800; }"
        ".day-icon { color: #c6b575; font-size: 16px; }"
        ".today-label { color: #8fbf73; font-weight: 800; }"
        ".updated { color: #93aaa0; font-size: 11px; }"
        ".close { border-radius: 999px; padding: 0; min-width: 32px; min-height: 32px; background: #182722; color: #93aaa0; border: 1px solid #253831; font-size: 16px; }"
        ".close:hover { background: #253831; color: #dbe6dc; }";

    GtkCssProvider * provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, css, -1);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);

    GtkWidget * win = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(win), "Weather");
    gtk_window_set_default_size(GTK_WINDOW(win), 640, 620);
    gtk_window_set_resizable(GTK_WINDOW(win), FALSE);
    gtk_window_set_decorated(GTK_WINDOW(win), FALSE);

    GtkWidget * root = weather_panel();
    gtk_window_set_child(GTK_WINDOW(win), root);

    GtkWidget * close = gtk_widget_get_last_child(gtk_widget_get_first_child(root));
    g_signal_connect_swapped(close, "clicked", G_CALLBACK(gtk_window_destroy), win);

    GtkEventController * key = gtk_event_controller_key_new();
    g_signal_connect(key, "key-pressed", G_CALLBACK(on_key), win);
    gtk_widget_add_controller(win, key);

    gtk_window_present(GTK_WINDOW(win));
}

int main(int argc, char ** argv) {
    if (argc < 2) {
        g_printerr("usage: weather-panel <weather-cache.json>\n");
        return 2;
    }

    if (!load_weather(argv[1])) {
        return 1;
    }

    GtkApplication * app = gtk_application_new("local.waybar.weather", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), 0, NULL);
    g_object_unref(app);
    return status;
}
