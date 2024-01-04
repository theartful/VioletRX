use eframe::{
    egui::{Response, Sense, Ui, Widget},
    epaint::{Color32, Rounding, Vec2},
};

pub struct FreqCtrl {
    freq: i64,
    size: Vec2,
}

impl FreqCtrl {
    pub fn new(freq: i64, size: Vec2) -> Self {
        Self { freq, size }
    }
}

impl Widget for FreqCtrl {
    fn ui(self, ui: &mut Ui) -> Response {
        let response = ui.allocate_response(self.size, Sense::click_and_drag());

        let painter = ui.painter_at(response.rect);

        painter.rect_filled(response.rect, Rounding::default(), Color32::WHITE);

        response
    }
}
