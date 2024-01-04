use eframe::{
    egui::{
        Align2, Color32, Id, Painter, Pos2, Rect, Response, Sense, Shape, Stroke, Ui, Vec2, Widget,
    },
    epaint::{FontFamily, FontId},
};

use super::fftaverager::{DecimatedFftFrame, FftAverager};

pub struct Plotter {
    pub scene_viewport: Rect,
    pub max_viewport: Rect,
    pub min_viewport_size: Vec2,
    pub max_viewport_size: Vec2,

    grid_xunit: f64,
    grid_yunit: f64,
    grid_color: Color32,
    labels_color: Color32,

    x_spacing: f64,
    y_spacing: f64,
}

impl Plotter {
    const MARGIN_WIDTH: i32 = 35;
    const MARGIN_HEIGHT: i32 = 30;

    pub fn new() -> Self {
        Self {
            scene_viewport: Rect::from_min_max(Pos2::new(87.5e6, -180.0), Pos2::new(112.5e6, 10.0)),
            max_viewport: Rect::from_min_max(Pos2::new(0.0, -180.0), Pos2::new(0.0, 10.0)),
            min_viewport_size: Vec2::new(10e3, 10.0),
            max_viewport_size: Vec2::new(100e8, 200.0),
            grid_xunit: 1e6,
            grid_yunit: 1.0,
            grid_color: Color32::from_rgb(0x44, 0x42, 0x42),
            labels_color: Color32::WHITE,
            x_spacing: 100.0,
            y_spacing: 50.0,
        }
    }

    fn map_to_scene(&self, pixel_viewport: Rect, pos: Pos2) -> Pos2 {
        let scale_x = self.scene_viewport.width() / pixel_viewport.width();
        let scale_y = self.scene_viewport.height() / pixel_viewport.height();

        Pos2::new(
            (pos.x - pixel_viewport.left()) * scale_x + self.scene_viewport.left(),
            (pixel_viewport.bottom() - pos.y) * scale_y + self.scene_viewport.top(),
        )
    }

    fn map_to_screen(&self, pixel_viewport: Rect, pos: Pos2) -> Pos2 {
        let scale_x = pixel_viewport.width() / self.scene_viewport.width();
        let scale_y = pixel_viewport.height() / self.scene_viewport.height();

        Pos2::new(
            (pos.x - self.scene_viewport.left()) * scale_x + pixel_viewport.left(),
            (self.scene_viewport.bottom() - pos.y) * scale_y + pixel_viewport.top(),
        )
    }

    pub fn translate_viewport(&mut self, delta: Vec2) {
        self.set_viewport(self.scene_viewport.translate(delta));
    }

    pub fn set_viewport(&mut self, mut viewport: Rect) {
        // enforce min and max viewport sizes
        if viewport.width() < self.min_viewport_size.x {
            viewport = viewport.expand2(Vec2::new(
                (self.min_viewport_size.x - viewport.width()) / 2.0,
                0.0,
            ));
        } else if viewport.width() > self.max_viewport_size.x {
            viewport = viewport.shrink2(Vec2::new(
                (viewport.width() - self.max_viewport_size.x) / 2.0,
                0.0,
            ));
        }

        if viewport.height() < self.min_viewport_size.y {
            viewport = viewport.expand2(Vec2::new(
                0.0,
                (self.min_viewport_size.y - viewport.height()) / 2.0,
            ));
        } else if viewport.height() > self.max_viewport_size.y {
            viewport = viewport.shrink2(Vec2::new(
                0.0,
                (viewport.height() - self.max_viewport_size.y) / 2.0,
            ));
        }

        let mut tx = 0.0;
        let mut ty = 0.0;

        if self.max_viewport.width() > 0.0 {
            if viewport.width() > self.max_viewport.width() {
                viewport.set_left(self.max_viewport.left());
                viewport.set_right(self.max_viewport.right());
            }

            if viewport.left() < self.max_viewport.left() {
                tx = self.max_viewport.left() - viewport.left();
            } else if viewport.right() > self.max_viewport.right() {
                tx = self.max_viewport.right() - viewport.right();
            }
        }

        if self.max_viewport.height() > 0.0 {
            if viewport.height() > self.max_viewport.height() {
                viewport.set_top(self.max_viewport.top());
                viewport.set_bottom(self.max_viewport.bottom());
            }

            if viewport.top() < self.max_viewport.top() {
                ty = self.max_viewport.top() - viewport.top();
            } else if viewport.bottom() > self.max_viewport.bottom() {
                ty = self.max_viewport.bottom() - viewport.bottom();
            }
        }
        viewport = viewport.translate(Vec2::new(tx, ty));

        self.scene_viewport = viewport;
    }

    pub fn set_max_viewport(&mut self, viewport: Rect) {
        self.max_viewport = viewport;
    }

    fn draw_grid(&self, response: &Response, painter: &Painter) {
        // paint grid
        let scale_x = self.scene_viewport.width() as f64 / response.rect.width() as f64;
        let scale_y = self.scene_viewport.height() as f64 / response.rect.height() as f64;

        let x_unit = {
            let max_x_len = self.x_spacing * scale_x;

            // 2^power2 * 5^power5 * grid_unit = max_x_len
            let x_power5 = (max_x_len / self.grid_xunit).log(5.0) as i32;
            let pow5l = 5.0_f64.powi(x_power5);
            let x_power2 = (max_x_len / (self.grid_xunit * pow5l)).log2() as i32;
            let pow2l = 2.0_f64.powi(x_power2);

            pow2l * pow5l * self.grid_xunit
        };

        let y_unit = {
            let max_y_len = self.y_spacing * scale_y;

            // 2^power2 * 5^power5 * grid_unit = max_y_len
            let y_power5 = (max_y_len / self.grid_yunit).log(5.0) as i32;
            let pow5l = 5.0_f64.powi(y_power5);
            let y_power2 = (max_y_len / (self.grid_yunit * pow5l)).log2() as i32;
            let pow2l = 2.0_f64.powi(y_power2);

            pow2l * pow5l * self.grid_yunit
        };

        // FIXME: margins should be calculated!
        let margin_x = Plotter::MARGIN_WIDTH as f64 * scale_x;
        let margin_y = Plotter::MARGIN_HEIGHT as f64 * scale_y;

        let left = ((self.scene_viewport.left() as f64 + margin_x) / x_unit).ceil() * x_unit;
        let num_steps = ((self.scene_viewport.right() as f64 - left) / x_unit) as i32;

        for i in 0..=num_steps {
            let x = left + x_unit * i as f64;

            let x_pixel = (x - self.scene_viewport.left() as f64) / scale_x;
            let path = [
                Pos2::new(x_pixel as f32, response.rect.top()),
                Pos2::new(
                    x_pixel as f32,
                    (response.rect.bottom() - Plotter::MARGIN_HEIGHT as f32)
                        .max(response.rect.top()),
                ),
            ];

            painter.add(Shape::dashed_line(
                &path,
                Stroke::new(1.0, self.grid_color),
                2.0,
                1.0,
            ));

            painter.text(
                Pos2::new(
                    x_pixel as f32,
                    (response.rect.bottom() - Plotter::MARGIN_HEIGHT as f32 / 2.0)
                        .max(response.rect.top()),
                ),
                Align2::CENTER_CENTER,
                (x / self.grid_xunit).to_string(),
                FontId::new(14.0, FontFamily::Proportional),
                self.labels_color,
            );
        }

        let top = ((self.scene_viewport.top() as f64 + margin_y) / y_unit).ceil() * y_unit;
        let num_steps = ((self.scene_viewport.bottom() as f64 - top) / y_unit) as i32;

        for i in 0..=num_steps {
            let y = top + y_unit * i as f64;

            let y_pixel =
                response.rect.bottom() as f64 - (y - self.scene_viewport.top() as f64) / scale_y;

            let path = [
                Pos2::new(
                    (response.rect.left() + Plotter::MARGIN_WIDTH as f32)
                        .min(response.rect.right()),
                    y_pixel as f32,
                ),
                Pos2::new(response.rect.right(), y_pixel as f32),
            ];

            painter.add(Shape::dashed_line(
                &path,
                Stroke::new(1.0, self.grid_color),
                2.0,
                2.0,
            ));

            painter.text(
                Pos2::new(
                    (response.rect.left() + Plotter::MARGIN_WIDTH as f32 / 2.0)
                        .min(response.rect.right()),
                    y_pixel as f32,
                ),
                Align2::CENTER_CENTER,
                (y / self.grid_yunit).to_string(),
                FontId::new(14.0, FontFamily::Proportional),
                self.labels_color,
            );
        }
    }

    pub fn draw_fft(&self, response: &Response, painter: &Painter, frame: &DecimatedFftFrame) {
        if frame.data.is_empty() {
            return;
        }
        let map_to_screen = |pos: Pos2| self.map_to_screen(response.rect, pos);

        let mut prev_pos = map_to_screen(Pos2::new(frame.left, frame.data[0]));

        for i in 1..frame.data.len() {
            let pos = map_to_screen(Pos2::new(
                frame.left + (frame.right - frame.left) * i as f32 / (frame.data.len() - 1) as f32,
                frame.data[i],
            ));

            painter.line_segment([prev_pos, pos], Stroke::new(1.0, Color32::WHITE));
            prev_pos = pos;
        }
    }

    pub fn zoom_x(&mut self, factor: f32, x: f32) {
        let mut viewport = self.scene_viewport;
        let d_width = factor * viewport.width();
        viewport.set_left(
            viewport.left() - d_width * (x - viewport.left()) / self.scene_viewport.width(),
        );
        viewport.set_right(
            viewport.right() + d_width * (viewport.right() - x) / self.scene_viewport.width(),
        );

        self.set_viewport(viewport);
    }

    pub fn zoom_y(&mut self, factor: f32, y: f32) {
        let mut viewport = self.scene_viewport;
        let d_height = factor * viewport.height();
        viewport.set_top(
            viewport.top() - d_height * (y - viewport.top()) / self.scene_viewport.height(),
        );
        viewport.set_bottom(
            viewport.bottom() + d_height * (viewport.bottom() - y) / self.scene_viewport.height(),
        );

        self.set_viewport(viewport);
    }
}

impl Widget for &mut Plotter {
    fn ui(self, ui: &mut Ui) -> Response {
        let response = ui.allocate_response(ui.available_size(), Sense::click_and_drag());

        let painter = ui.painter_at(response.rect);
        self.draw_grid(&response, &painter);

        let xdrag_rect = Rect::from_min_max(
            Pos2::new(
                response.rect.left(),
                response.rect.bottom() - Plotter::MARGIN_HEIGHT as f32,
            ),
            Pos2::new(response.rect.right(), response.rect.bottom()),
        );
        let ydrag_rect = Rect::from_min_max(
            Pos2::new(response.rect.left(), response.rect.top()),
            Pos2::new(
                response.rect.left() + Plotter::MARGIN_WIDTH as f32,
                response.rect.bottom(),
            ),
        );
        let grid_rect = Rect::from_min_max(
            Pos2::new(
                response.rect.left() + Plotter::MARGIN_WIDTH as f32,
                response.rect.top(),
            ),
            Pos2::new(
                response.rect.right(),
                response.rect.bottom() - Plotter::MARGIN_HEIGHT as f32,
            ),
        );

        let xdrag_id = response.id.with("xdrag");
        let ydrag_id = response.id.with("ydrag");
        let grid_id = response.id.with("grid");

        let xdrag_response = ui.interact(xdrag_rect, xdrag_id, Sense::click_and_drag());
        let ydrag_response = ui.interact(ydrag_rect, ydrag_id, Sense::click_and_drag());
        let grid_response = ui.interact(grid_rect, grid_id, Sense::hover());

        if xdrag_response.dragged() {
            let scale_x = self.scene_viewport.width() / response.rect.width();
            self.translate_viewport(xdrag_response.drag_delta() * Vec2::new(-scale_x, 0.0));

            // change cursor
            ui.ctx().set_cursor_icon(eframe::egui::CursorIcon::Grabbing);
        } else if ydrag_response.dragged() {
            let scale_y = self.scene_viewport.height() / response.rect.height();
            self.translate_viewport(ydrag_response.drag_delta() * Vec2::new(0.0, scale_y));

            // change cursor
            ui.ctx().set_cursor_icon(eframe::egui::CursorIcon::Grabbing);
        } else if grid_response.hovered() {
            if let Some(pixel_pos) = grid_response.hover_pos() {
                let pos = self.map_to_scene(response.rect, pixel_pos);
                grid_response.on_hover_text_at_pointer(format!(
                    "{:<10} kHZ\n{:<10.2} dB",
                    (pos.x / 1000.0) as i32,
                    pos.y
                ));
            }
        }

        if let Some(pos) = ui.ctx().input(|input| input.pointer.hover_pos()) {
            let delta = ui.ctx().input(|input| input.scroll_delta.y);
            if delta != 0.0 {
                let factor = if delta > 0.0 { -0.08 } else { 0.08 };
                let pos = self.map_to_scene(response.rect, pos);

                if ui.rect_contains_pointer(xdrag_rect) {
                    self.zoom_x(factor, pos.x);
                } else if ui.rect_contains_pointer(ydrag_rect) {
                    self.zoom_y(factor, pos.y);
                }
            }
        }

        response
    }
}
