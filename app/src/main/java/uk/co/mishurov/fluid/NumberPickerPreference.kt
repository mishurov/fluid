package uk.co.mishurov.fluid

import android.content.Context
import android.content.res.TypedArray
import android.preference.DialogPreference
import android.util.AttributeSet
import android.view.Gravity
import android.view.View
import android.view.ViewGroup
import android.widget.FrameLayout
import android.widget.NumberPicker


class NumberPickerPreference : DialogPreference {

    var mMin = DEFAULT_MIN
    var mMax = DEFAULT_MAX

    private var picker: NumberPicker? = null

    var value: Int = 0
        set(value) {
            field = value
            persistInt(value)
            setSummary(String.format(defaultSummary!!, value))
        }

    private var defaultSummary: String? = null

    constructor(context: Context, attrs: AttributeSet) : super(context, attrs) {
        init_picker(attrs)
    }

    constructor(context: Context, attrs: AttributeSet, defStyleAttr: Int)
                                        : super(context, attrs, defStyleAttr) {
        init_picker(attrs)
    }

    override fun onCreateDialogView(): View {
        val layoutParams = FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
        )
        layoutParams.gravity = Gravity.CENTER

        picker = NumberPicker(getContext())
        picker?.setLayoutParams(layoutParams)

        val dialogView = FrameLayout(getContext())
        dialogView.addView(picker)

        return dialogView
    }

    private fun init_picker(attrs: AttributeSet) {
        mMin = attrs.getAttributeIntValue(null, "min", DEFAULT_MIN)
        mMax = attrs.getAttributeIntValue(null, "max", DEFAULT_MAX)
        defaultSummary = getSummary().toString()
    }

    override fun onBindDialogView(view: View) {
        super.onBindDialogView(view)
        picker?.setMinValue(mMin)
        picker?.setMaxValue(mMax)
        picker?.setWrapSelectorWheel(WRAP_SELECTOR_WHEEL)
        picker?.setValue(value)
    }

    override fun onDialogClosed(positiveResult: Boolean) {
        if (positiveResult) {
            picker!!.clearFocus()
            val newValue = picker!!.getValue()
            if (callChangeListener(newValue)) {
                value = newValue
            }
        }
    }

    override fun onGetDefaultValue(a: TypedArray, index: Int): Int {
        return a.getInt(index, mMin)
    }

    override fun onSetInitialValue(restoreValue: Boolean, defaultValue: Any?) {
        if (restoreValue) {
            value = getPersistedInt(mMin)
        } else {
            value = defaultValue as Int
        }
    }

    companion object {

        // allowed range
        val DEFAULT_MIN = 0
        val DEFAULT_MAX = 100
        // enable or disable the 'circular behavior'
        val WRAP_SELECTOR_WHEEL = true
    }
}
