#include "Theme.h"
#include "UILayout.h"
#include "UIWindow.h"
#include "UILabel.h"
#include <numeric>

namespace synui
{
	BoxLayout::BoxLayout(Orientation orientation, Alignment alignment,
		int margin, int spacing)
		: mOrientation(orientation), mAlignment(alignment), mMargin(margin),
		mSpacing(spacing) {
	}

	Vector2i BoxLayout::preferredSize(NVGcontext *ctx, const UIComponent *widget) const {
		Vector2i size = Vector2i::Constant(2 * mMargin);

		if (dynamic_cast<const UIWindow *>(widget))
			size[1] += widget->theme()->mWindowHeaderHeight - mMargin / 2;

		bool first = true;
		int axis1 = (int)mOrientation, axis2 = ((int)mOrientation + 1) % 2;
		for (auto w : widget->children()) {
			if (!w->visible())
				continue;
			if (first)
				first = false;
			else
				size[axis1] += mSpacing;

			Vector2i ps = w->minSize(), fs = w->size();
			Vector2i targetSize = ps.cwiseMax(fs);

			size[axis1] += targetSize[axis1];
			size[axis2] = syn::MAX(size[axis2], targetSize[axis2] + 2 * mMargin);
			first = false;
		}
		return size;
	}

	void BoxLayout::performLayout(NVGcontext *ctx, UIComponent *widget) const {
		Vector2i fs_w = widget->size();
		Vector2i containerSize(
			fs_w[0] ? fs_w[0] : widget->size().x(),
			fs_w[1] ? fs_w[1] : widget->size().y()
		);

		int axis1 = (int)mOrientation, axis2 = ((int)mOrientation + 1) % 2;
		int position = mMargin;

		if (dynamic_cast<UIWindow *>(widget))
			position += widget->theme()->mWindowHeaderHeight - mMargin / 2;

		bool first = true;
		for (auto w : widget->children()) {
			if (!w->visible())
				continue;
			if (first)
				first = false;
			else
				position += mSpacing;

			Vector2i ps = w->minSize(), fs = w->size();
			Vector2i targetSize(
				fs[0] ? fs[0] : ps[0],
				fs[1] ? fs[1] : ps[1]
			);
			Vector2i pos = Vector2i::Zero();
			pos[axis1] = position;

			switch (mAlignment) {
			case Alignment::Minimum:
				pos[axis2] = mMargin;
				break;
			case Alignment::Middle:
				pos[axis2] = (containerSize[axis2] - targetSize[axis2]) / 2;
				break;
			case Alignment::Maximum:
				pos[axis2] = containerSize[axis2] - targetSize[axis2] - mMargin;
				break;
			case Alignment::Fill:
				pos[axis2] = mMargin;
				targetSize[axis2] = fs[axis2] ? fs[axis2] : containerSize[axis2];
				break;
			}

			w->setRelPos(pos);
			w->setSize(targetSize);
			w->performLayout(ctx);
			position += targetSize[axis1];
		}
	}

	Vector2i GroupLayout::preferredSize(NVGcontext *ctx, const UIComponent *widget) const {
		int height = mMargin, width = 2 * mMargin;

		const UIWindow *window = dynamic_cast<const UIWindow *>(widget);
		if (window)
			height += widget->theme()->mWindowHeaderHeight - mMargin / 2;

		bool first = true, indent = false;
		for (auto c : widget->children()) {
			if (!c->visible())
				continue;
			const UILabel *label = dynamic_cast<const UILabel *>(c.get());
			if (!first)
				height += (label == nullptr) ? mSpacing : mGroupSpacing;
			first = false;

			Vector2i ps = c->minSize(), fs = c->size();
			Vector2i targetSize(
				fs[0] ? fs[0] : ps[0],
				fs[1] ? fs[1] : ps[1]
			);

			bool indentCur = indent && label == nullptr;
			height += targetSize.y();
			width = syn::MAX(width, targetSize.x() + 2 * mMargin + (indentCur ? mGroupIndent : 0));

			if (label)
				indent = !label->text().empty();
		}
		height += mMargin;
		return Vector2i(width, height);
	}

	void GroupLayout::performLayout(NVGcontext *ctx, UIComponent *widget) const {
		int height = mMargin, availableWidth = widget->size().x() - 2 * mMargin;

		const UIWindow *window = dynamic_cast<const UIWindow *>(widget);
		if (window)
			height += widget->theme()->mWindowHeaderHeight - mMargin / 2;

		bool first = true, indent = false;
		for (auto c : widget->children()) {
			if (!c->visible())
				continue;
			const UILabel *label = dynamic_cast<const UILabel *>(c.get());
			if (!first)
				height += (label == nullptr) ? mSpacing : mGroupSpacing;
			first = false;

			bool indentCur = indent && label == nullptr;
			Vector2i ps = Vector2i(availableWidth - (indentCur ? mGroupIndent : 0),
				c->minSize().y());
			Vector2i fs = c->size();

			Vector2i targetSize(
				fs[0] ? fs[0] : ps[0],
				fs[1] ? fs[1] : ps[1]
			);

			c->setRelPos(Vector2i(mMargin + (indentCur ? mGroupIndent : 0), height));
			c->setSize(targetSize);
			c->performLayout(ctx);

			height += targetSize.y();

			if (label)
				indent = !label->text().empty();
		}
	}

	Vector2i GridLayout::preferredSize(NVGcontext *ctx,
		const UIComponent *widget) const {
		/* Compute minimum row / column sizes */
		std::vector<int> grid[2];
		computeLayout(ctx, widget, grid);

		Vector2i size(
			2 * mMargin + std::accumulate(grid[0].begin(), grid[0].end(), 0)
			+ syn::MAX((int)grid[0].size() - 1, 0) * mSpacing[0],
			2 * mMargin + std::accumulate(grid[1].begin(), grid[1].end(), 0)
			+ syn::MAX((int)grid[1].size() - 1, 0) * mSpacing[1]
		);

		if (dynamic_cast<const UIWindow *>(widget))
			size[1] += widget->theme()->mWindowHeaderHeight - mMargin / 2;

		return size;
	}

	void GridLayout::computeLayout(NVGcontext *ctx, const UIComponent *widget, std::vector<int> *grid) const {
		int axis1 = (int)mOrientation, axis2 = (axis1 + 1) % 2;
		size_t numChildren = widget->children().size(), visibleChildren = 0;
		for (auto w : widget->children())
			visibleChildren += w->visible() ? 1 : 0;

		Vector2i dim;
		dim[axis1] = mResolution;
		dim[axis2] = (int)((visibleChildren + mResolution - 1) / mResolution);

		grid[axis1].clear(); grid[axis1].resize(dim[axis1], 0);
		grid[axis2].clear(); grid[axis2].resize(dim[axis2], 0);

		size_t child = 0;
		for (int i2 = 0; i2 < dim[axis2]; i2++) {
			for (int i1 = 0; i1 < dim[axis1]; i1++) {
				UIComponent *w = nullptr;
				do {
					if (child >= numChildren)
						return;
					w = widget->children()[child++].get();
				} while (!w->visible());

				Vector2i ps = w->minSize();
				Vector2i fs = w->size();
				Vector2i targetSize(
					fs[0] ? fs[0] : ps[0],
					fs[1] ? fs[1] : ps[1]
				);

				grid[axis1][i1] = syn::MAX(grid[axis1][i1], targetSize[axis1]);
				grid[axis2][i2] = syn::MAX(grid[axis2][i2], targetSize[axis2]);
			}
		}
	}

	void GridLayout::performLayout(NVGcontext *ctx, UIComponent *widget) const {
		Vector2i fs_w = widget->size();
		Vector2i containerSize(
			fs_w[0] ? fs_w[0] : widget->size().x(),
			fs_w[1] ? fs_w[1] : widget->size().y()
		);

		/* Compute minimum row / column sizes */
		std::vector<int> grid[2];
		computeLayout(ctx, widget, grid);
		int dim[2] = { (int)grid[0].size(), (int)grid[1].size() };

		Vector2i extra = Vector2i::Zero();
		if (dynamic_cast<UIWindow *>(widget))
			extra[1] += widget->theme()->mWindowHeaderHeight - mMargin / 2;

		/* Strech to size provided by \c widget */
		for (int i = 0; i < 2; i++) {
			int gridSize = 2 * mMargin + extra[i];
			for (int s : grid[i]) {
				gridSize += s;
				if (i + 1 < dim[i])
					gridSize += mSpacing[i];
			}

			if (gridSize < containerSize[i]) {
				/* Re-distribute remaining space evenly */
				int gap = containerSize[i] - gridSize;
				int g = gap / dim[i];
				int rest = gap - g * dim[i];
				for (int j = 0; j < dim[i]; ++j)
					grid[i][j] += g;
				for (int j = 0; rest > 0 && j < dim[i]; --rest, ++j)
					grid[i][j] += 1;
			}
		}

		int axis1 = (int)mOrientation, axis2 = (axis1 + 1) % 2;
		Vector2i start = Vector2i::Constant(mMargin) + extra;

		size_t numChildren = widget->children().size();
		size_t child = 0;

		Vector2i pos = start;
		for (int i2 = 0; i2 < dim[axis2]; i2++) {
			pos[axis1] = start[axis1];
			for (int i1 = 0; i1 < dim[axis1]; i1++) {
				UIComponent *w = nullptr;
				do {
					if (child >= numChildren)
						return;
					w = widget->children()[child++].get();
				} while (!w->visible());

				Vector2i ps = w->minSize();
				Vector2i fs = w->size();
				Vector2i targetSize(
					fs[0] ? fs[0] : ps[0],
					fs[1] ? fs[1] : ps[1]
				);

				Vector2i itemPos(pos);
				for (int j = 0; j < 2; j++) {
					int axis = (axis1 + j) % 2;
					int item = j == 0 ? i1 : i2;
					Alignment align = alignment(axis, item);

					switch (align) {
					case Alignment::Minimum:
						break;
					case Alignment::Middle:
						itemPos[axis] += (grid[axis][item] - targetSize[axis]) / 2;
						break;
					case Alignment::Maximum:
						itemPos[axis] += grid[axis][item] - targetSize[axis];
						break;
					case Alignment::Fill:
						targetSize[axis] = fs[axis] ? fs[axis] : grid[axis][item];
						break;
					}
				}
				w->setRelPos(itemPos);
				w->setSize(targetSize);
				w->performLayout(ctx);
				pos[axis1] += grid[axis1][i1] + mSpacing[axis1];
			}
			pos[axis2] += grid[axis2][i2] + mSpacing[axis2];
		}
	}

	AdvancedGridLayout::AdvancedGridLayout(const std::vector<int> &cols, const std::vector<int> &rows)
		: mCols(cols), mRows(rows) {
		mColStretch.resize(mCols.size(), 0);
		mRowStretch.resize(mRows.size(), 0);
	}

	Vector2i AdvancedGridLayout::preferredSize(NVGcontext *ctx, const UIComponent *widget) const {
		/* Compute minimum row / column sizes */
		std::vector<int> grid[2];
		computeLayout(ctx, widget, grid);

		Vector2i size(
			std::accumulate(grid[0].begin(), grid[0].end(), 0),
			std::accumulate(grid[1].begin(), grid[1].end(), 0));

		Vector2i extra = Vector2i::Constant(2 * mMargin);
		if (dynamic_cast<const UIWindow *>(widget))
			extra[1] += widget->theme()->mWindowHeaderHeight - mMargin / 2;

		return size + extra;
	}

	void AdvancedGridLayout::performLayout(NVGcontext *ctx, UIComponent *widget) const {
		std::vector<int> grid[2];
		computeLayout(ctx, widget, grid);

		grid[0].insert(grid[0].begin(), mMargin);
		if (dynamic_cast<const UIWindow *>(widget))
			grid[1].insert(grid[1].begin(), widget->theme()->mWindowHeaderHeight + mMargin / 2);
		else
			grid[1].insert(grid[1].begin(), mMargin);

		for (int axis = 0; axis<2; ++axis) {
			for (size_t i = 1; i<grid[axis].size(); ++i)
				grid[axis][i] += grid[axis][i - 1];

			for (shared_ptr<UIComponent> w : widget->children()) {
				Anchor anchor = this->anchor(w.get());
				if (!w->visible())
					continue;

				int itemPos = grid[axis][anchor.pos[axis]];
				int cellSize = grid[axis][anchor.pos[axis] + anchor.size[axis]] - itemPos;
				int ps = w->minSize()[axis], fs = w->size()[axis];
				int targetSize = fs ? fs : ps;

				switch (anchor.align[axis]) {
				case Alignment::Minimum:
					break;
				case Alignment::Middle:
					itemPos += (cellSize - targetSize) / 2;
					break;
				case Alignment::Maximum:
					itemPos += cellSize - targetSize;
					break;
				case Alignment::Fill:
					targetSize = fs ? fs : cellSize;
					break;
				}

				Vector2i pos = w->getRelPos(), size = w->size();
				pos[axis] = itemPos;
				size[axis] = targetSize;
				w->setRelPos(pos);
				w->setSize(size);
				w->performLayout(ctx);
			}
		}
	}

	void AdvancedGridLayout::computeLayout(NVGcontext *ctx, const UIComponent *widget,
		std::vector<int> *_grid) const {
		Vector2i fs_w = widget->size();
		Vector2i containerSize(
			fs_w[0] ? fs_w[0] : widget->size().x(),
			fs_w[1] ? fs_w[1] : widget->size().y()
		);

		Vector2i extra = Vector2i::Constant(2 * mMargin);
		if (dynamic_cast<const UIWindow *>(widget))
			extra[1] += widget->theme()->mWindowHeaderHeight - mMargin / 2;

		containerSize -= extra;

		for (int axis = 0; axis<2; ++axis) {
			std::vector<int> &grid = _grid[axis];
			const std::vector<int> &sizes = axis == 0 ? mCols : mRows;
			const std::vector<float> &stretch = axis == 0 ? mColStretch : mRowStretch;
			grid = sizes;

			for (int phase = 0; phase < 2; ++phase) {
				for (auto pair : mAnchor) {
					const UIComponent *w = pair.first;
					if (!w->visible())
						continue;
					const Anchor &anchor = pair.second;
					if ((anchor.size[axis] == 1) != (phase == 0))
						continue;
					int ps = w->minSize()[axis], fs = w->size()[axis];
					int targetSize = fs ? fs : ps;

					if (anchor.pos[axis] + anchor.size[axis] > (int)grid.size())
						throw std::runtime_error(
							"Advanced grid layout: widget is out of bounds: " +
							(std::string) anchor);

					int currentSize = 0;
					float totalStretch = 0;
					for (int i = anchor.pos[axis];
						i < anchor.pos[axis] + anchor.size[axis]; ++i) {
						if (sizes[i] == 0 && anchor.size[axis] == 1)
							grid[i] = syn::MAX(grid[i], targetSize);
						currentSize += grid[i];
						totalStretch += stretch[i];
					}
					if (targetSize <= currentSize)
						continue;
					if (totalStretch == 0)
						throw std::runtime_error(
							"Advanced grid layout: no space to place widget: " +
							(std::string) anchor);
					float amt = (targetSize - currentSize) / totalStretch;
					for (int i = anchor.pos[axis];
						i < anchor.pos[axis] + anchor.size[axis]; ++i) {
						grid[i] += (int)std::round(amt * stretch[i]);
					}
				}
			}

			int currentSize = std::accumulate(grid.begin(), grid.end(), 0);
			float totalStretch = std::accumulate(stretch.begin(), stretch.end(), 0.0f);
			if (currentSize >= containerSize[axis] || totalStretch == 0)
				continue;
			float amt = (containerSize[axis] - currentSize) / totalStretch;
			for (size_t i = 0; i<grid.size(); ++i)
				grid[i] += (int)std::round(amt * stretch[i]);
		}
	}
}