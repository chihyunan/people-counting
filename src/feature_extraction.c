#include "feature_extraction.h"
static uint32_t calc_centroid(uint8_t *mask, int width, int height, uint32_t *centroid_index);
static int calc_centrals(uint8_t *submask, int width, int height, uint32_t **location_list_ptr, uint8_t **distance_list_ptr);
void chamfer_distance_transform(uint8_t *mask, int width, int height, int a, int b);
int central_detector(uint8_t *input, uint8_t *output, int const img_w, int const img_h);

void print_blob_info(Blob *blob_list, int n_blobs)
{
    for (int i = 0; i < n_blobs; i++)
    {
        printf("blob num %d: ", i + 1);
        printf("size %d centroid %d\n", blob_list[i].size, blob_list[i].centroid_index);
        printf("central number %d\n", blob_list[i].n_central_points);
    }
}

void print_blob_mask(Blob *blob_list, int n_blobs, int w, int h)
{
    uint8_t *holder = malloc(sizeof(uint8_t) * (w * h));
    for (int n = 0; n < n_blobs; n++)
    {
        for (int i = 0; i < (w * h); i++)
        {
            holder[i] = 0;
        }
        printf("blob number %d: \n", n + 1);
        for (int i = 0; i < blob_list[n].n_central_points; i++)
        {
            int index = blob_list[n].central_index_list[i];
            holder[index] = blob_list[n].central_distance_list[i];
        }
        for (int r = 0; r < h; r++)
        {
            for (int c = 0; c < w; c++)
            {
                int index = r * w + c;
                if (holder[index])
                {
                    printf("%d ", holder[r * w + c]);
                }
                else
                {
                    printf("- ");
                }
            }
            printf("\n");
        }
    }
    free(holder);
}

struct Blob *extract_feature(uint8_t *labeled, int n_blobs, int img_w, int img_h)
{
    if (n_blobs == 0)
    {
        return NULL;
    }
    struct Blob *blob_list = malloc(sizeof(struct Blob) * n_blobs);
    if (blob_list == NULL)
    {
        printf("extract_feature malloc failed\n");
        return NULL;
    }
    for (int num = 1; num <= n_blobs; num++)
    {
        uint8_t *submask = malloc(sizeof(uint8_t) * (img_w * img_h));
        if (submask == NULL)
        {
            printf("extract feature malloc failed\n");
            free(blob_list);
            return NULL;
        }
        for (int i = 0; i < (img_w * img_h); i++)
        {
            submask[i] = (labeled[i] == num) ? 1 : 0;
        }
        blob_list[num - 1].size = calc_centroid(submask, img_w, img_h, &blob_list[num - 1].centroid_index);
        blob_list[num - 1].n_central_points = calc_centrals(submask, img_w, img_h, &blob_list[num - 1].central_index_list, &blob_list[num - 1].central_distance_list);
        free(submask);
    }
    return blob_list;
}

/** @brief calculate central points of a single blob
 * @param submask,width,height the binary mask of a single blob and its dimensions
 * @param location_list_ptr a pointer to a list storing the linear index of central points
 * @param distance_list_ptr a pointer to a list storing the distance value of central points 
 * @return length of the list, -1 when error
 */
static int calc_centrals(uint8_t *submask, int width, int height, uint32_t **location_list_ptr, uint8_t **distance_list_ptr)
{
    static uint8_t mask_holder[IM_LEN];
    chamfer_dt_city(submask, width, height);
    int n_centrals = central_detector(submask, mask_holder, width, height);
    if (n_centrals == 0)
    {
        *location_list_ptr = NULL;
        *distance_list_ptr = NULL;
        return 0;
    }
    *location_list_ptr = malloc(sizeof(uint32_t) * n_centrals);
    *distance_list_ptr = malloc(sizeof(uint8_t) * n_centrals);
    int count = 0;
    for (int i = 0; i < (width * height); i++)
    {
        if (mask_holder[i] == 0)
        {
            continue;
        }
        (*location_list_ptr)[count] = i;
        (*distance_list_ptr)[count] = mask_holder[i];
        count += 1;
    }

    return n_centrals;
}

void chamfer_dt_city(uint8_t *mask, int width, int height)
{
    chamfer_distance_transform(mask, width, height, 1, 2);
}

/** @brief chamfer distance transform of a single blob
 * @param mask,width,height the binary mask to be transformed and its width, height
 * @param a,b the distance definition, a: distance to neighbour-4 pixels, b: distance to corner-neighbour pixels 
*/
void chamfer_distance_transform(uint8_t *mask, int width, int height, int a, int b)
{
    if ((width >= 255) || (height >= 255))
    {
        printf("WARN: distance is hold in a uint8_t variable, which may overflow \n \
        for large image when object dimension could be greater than 255\n");
    }

    //first pass
    for (int r = 0; r < height; r++)
    {
        for (int c = 0; c < width; c++)
        {
            int index = r * width + c;
            if (mask[index])
            {
                uint32_t candidates[4];
                int n_candidates = 0;
                if (r >= 1)
                {
                    candidates[n_candidates++] = mask[index - width] + a; //north
                    if (c >= 1)
                    {
                        candidates[n_candidates++] = mask[index - width - 1] + b; //north west
                    }
                    if (c < (width - 1))
                    {
                        candidates[n_candidates++] = mask[index - width + 1] + b; //north east
                    }
                }
                if (c >= 1)
                {
                    candidates[n_candidates++] = mask[index - 1] + a; //west
                }
                if (!n_candidates)
                {
                    mask[index] = 1;
                }
                else
                {
                    int min_loc = 0;
                    for (int i = 1; i < n_candidates; i++)
                    {
                        if (candidates[i] < candidates[min_loc])
                        {
                            min_loc = i;
                        }
                    }
                    mask[index] = candidates[min_loc];
                }
            }
        }
    }
    //second pass
    for (int r = (height - 1); r >= 0; r--)
    {
        for (int c = (width - 1); c >= 0; c--)
        {
            int index = r * width + c;
            if (mask[index])
            {
                uint32_t candidates[5];
                candidates[0] = mask[index];
                int n_candidates = 1;
                if (r < (height - 1))
                {
                    candidates[n_candidates++] = mask[index + width] + a; //south
                    if (c >= 1)
                    {
                        candidates[n_candidates++] = mask[index + width - 1] + b; //south west
                    }
                    if (c <= (width - 1))
                    {
                        candidates[n_candidates++] = mask[index + width + 1] + b; //south east
                    }
                }
                if (c <= (width - 1))
                {
                    candidates[n_candidates++] = mask[index + 1] + a; //east
                }
                if (!n_candidates)
                {
                    mask[index] = 1;
                }
                else
                {
                    int min_loc = 0;
                    for (int i = 1; i < n_candidates; i++)
                    {
                        if (candidates[i] < candidates[min_loc])
                        {
                            min_loc = i;
                        }
                    }
                    mask[index] = candidates[min_loc];
                }
            }
        }
    }
}

/** @brief calculate centroid of a single blob
 * @param mask,width,height the binary blob mask and its dimensions
 * @param index the vairable to store the linear index of the centroid
 * @return size of the blob
*/
static uint32_t calc_centroid(uint8_t *mask, int width, int height, uint32_t *centroid_index)
{
    uint32_t row_sum = 0;
    uint32_t col_sum = 0;
    uint32_t count = 0;
    for (int r = 0; r < height; r++)
    {
        for (int c = 0; c < width; c++)
        {
            int index = r * width + c;
            if (mask[index])
            {
                count++;
                col_sum += c;
                row_sum += r;
            }
        }
    }
    row_sum /= count;
    col_sum /= count;
    *centroid_index = row_sum * width + col_sum;
    return count;
}

void delete_blob_list(Blob *blob_list, int n_blobs)
{
    for (int i = 0; i < n_blobs; i++)
    {
        free(blob_list[i].central_distance_list);
        free(blob_list[i].central_index_list);
        blob_list[i].central_distance_list = NULL;
        blob_list[i].central_index_list = NULL;
    }
    free(blob_list);
    blob_list = NULL;
}

int central_detector(uint8_t *input, uint8_t *output, int const img_w, int const img_h)
{
    int n_centrals = 0;
    int kernel[] = {-1, -2, -1, -2, 12, -2, -1, -2, -1};
    //int kernel[] = {-1, -1, -1, -1, 8, -1, -1, -1, -1};
    int side = 3;
    int radius = (side - 1) / 2;
    int center = radius * side + radius;
    const int value_threshold = 3;
    const int count_threshold = 6;
    for (int i = 0; i < (img_w * img_h); i++)
    {
        output[i] = 0;
    }
    for (int r = 1; r < img_h - 1; r++)
    {
        for (int c = 1; c < img_w - 1; c++)
        {
            int px_index = r * img_w + c;
            if (input[px_index] <= value_threshold)
            {
                continue;
            }
            int sum = 0;
            for (int r_offset = -radius; r_offset <= radius; r_offset++)
            {
                int r_offset_fin = ((r + r_offset) < 0) ? (-r_offset) : r_offset;
                r_offset_fin = ((r + r_offset) >= img_h) ? (-r_offset) : r_offset;
                int index_offset_base = r_offset_fin * img_w;
                int kernel_offset_base = r_offset_fin * side;
                for (int c_offset = -radius; c_offset <= radius; c_offset++)
                {
                    int c_offset_fin = ((c + c_offset) < 0) ? (-c_offset) : c_offset;
                    c_offset_fin = ((c + c_offset) >= img_w) ? (-c_offset) : c_offset;
                    int index_offset = index_offset_base + c_offset_fin;
                    int kernel_offset = kernel_offset_base + c_offset_fin;
                    int inter = input[px_index + index_offset] * kernel[center + kernel_offset];
                    sum += inter;
                }
            }
            output[px_index] = ((sum >= count_threshold)) ? input[px_index] : 0;
            n_centrals += (sum >= count_threshold) ? 1 : 0;
        }
    }
    return n_centrals;
}