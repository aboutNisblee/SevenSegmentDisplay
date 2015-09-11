/**
 * \file sevensegmentdisplay.hpp
 *
 * \date 04.09.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef SEVENSEGMENTDISPLAY_HPP
#define SEVENSEGMENTDISPLAY_HPP

#include <memory>
#include <QQuickItem>

class DigitNodeSettings;

/**
 * QQuick widget that implements a seven-segment display.
 */
class SevenSegmentDisplay : public QQuickItem
{
    Q_OBJECT

	/** Property that controls the current value shown by the widget.
	 * @note Currently only values from 0 to 9 are supported. */
	Q_PROPERTY(int value READ getValue WRITE setValue NOTIFY valueChanged)
    /** Property that controls the digit height. */
	Q_PROPERTY(int digitSize READ getDigitSize WRITE setDigitSize NOTIFY digitSizeChanged)

	Q_ENUMS(Alignment)
	/** Property that controls the vertical alignment. */
	Q_PROPERTY(Alignment verticalAlignment READ getVerticalAlignment WRITE setVerticalAlignment NOTIFY verticalAlignmentChanged)
	/** Property that controls the horizontal alignment. */
	Q_PROPERTY(Alignment horizontalAlignment READ getHorizontalAlignment WRITE setHorizontalAlignment NOTIFY horizontalAlignmentChanged)

	/** Property that controls the background color. */
	Q_PROPERTY(QColor bgColor READ getBgColor WRITE setBgColor NOTIFY bgColorChanged)
	/** Property that controls the color of enabled segments. */
	Q_PROPERTY(QColor onColor READ getOnColor WRITE setOnColor NOTIFY onColorChanged)
	/** Property that controls the color of disabled segments. */
	Q_PROPERTY(QColor offColor READ getOffColor WRITE setOffColor NOTIFY offColorChanged)

public:
	enum Alignment
	{
		// TODO: Add some other alignments
		AlignCenter,
	};

    SevenSegmentDisplay(QQuickItem* parent = nullptr);

    int getValue() const;
    void setValue(int value);
    int getDigitSize() const;
    void setDigitSize(int size);
    Alignment getVerticalAlignment() const;
    void setVerticalAlignment(Alignment alignment);
    Alignment getHorizontalAlignment() const;
    void setHorizontalAlignment(Alignment alignment);
    QColor getBgColor() const;
    void setBgColor(const QColor& color);
    QColor getOnColor() const;
    void setOnColor(const QColor& color);
    QColor getOffColor() const;
    void setOffColor(const QColor& color);

signals:
	void valueChanged();
	void digitSizeChanged();
	void verticalAlignmentChanged();
	void horizontalAlignmentChanged();
	void bgColorChanged();
	void onColorChanged();
	void offColorChanged();

protected:
    QSGNode* updatePaintNode(QSGNode*, UpdatePaintNodeData*);

    /* TODO: Add the possibility to show multiple digits. */
    std::shared_ptr<DigitNodeSettings> mDigitSettings;
};

#endif // SEVENSEGMENTDISPLAY_HPP
